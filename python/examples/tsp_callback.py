from tsp_helpers import tsp_model, ford_fulkerson, UnionFind

from amplpy import AMPL
import amplpy_gurobi
import amplpy_cplex

ampls = amplpy_gurobi
var2tuple = ampls.var2tuple
tuple2var = ampls.tuple2var

VERBOSE = True
ENABLE_CALLBACK = True
ENABLE_MTZ = False
ENABLE_CB_MIPNODE = True

if ENABLE_MTZ:
    ENABLE_CB_MIPSOL = False  # not necessary if MTZ cuts are in place
else:
    ENABLE_CB_MIPSOL = True  # must add sub-tour elimination constraints

ampl = tsp_model('tsp_51_1.txt', ENABLE_MTZ)
m = ampl.exportModel("gurobi")
print("Model loaded, nvars=", m.getNumVars())

if ENABLE_CB_MIPSOL:  # needs lazy constraints
    m.enableLazyConstraints()

var_map = dict(m.getVarMapFiltered('x'))
xvars = {
    index: var2tuple(var)[1:]
    for var, index in var_map.items()}
vertices = list(sorted(set(
    [x[0] for x in xvars.values()] + [x[1] for x in xvars.values()]
)))


class MyCallback(ampls.GenericCallback):
    CALL_COUNT_MIPSOL = 0
    CALL_COUNT_MIPNODE = 0

    def mipsol(self):
        self.CALL_COUNT_MIPSOL += 1
        sol = self.getSolutionVector()
        nv = sum(abs(x) > 1e-5 for x in sol)
        if VERBOSE:
            print("MIPSOL #{}, nnz={}".format(self.CALL_COUNT_MIPSOL, nv))

        values = {
            xvars[i]: sol[i]
            for i in xvars
            if abs(sol[i]) > 1e-5
        }
        uf = UnionFind()
        for (u, v) in values:
            uf.link(u, v)
        groups = uf.groups()
        if len(groups) == 1:
            print('Valid solution!')
        else:
            for grp in groups:
                print('> sub-tour: ', grp)
                cutvarnames = [tuple2var('x', i, j)
                               for i in grp for j in grp if i != j]
                coeffs = [1 for i in range(len(cutvarnames))]
                self.addLazy(cutvarnames, coeffs,
                             ampls.CutDirection.LE, len(grp)-1)
        return 0

    def mipnode(self):
        self.CALL_COUNT_MIPNODE += 1
        if VERBOSE:
            print("MIPNODE #{}!".format(self.CALL_COUNT_MIPNODE))
        if self.CALL_COUNT_MIPNODE >= 1000:
            return 1
        sol = self.getSolutionVector()
        capacities = {
            xvars[i]: sol[i]
            for i in xvars
            if abs(sol[i]) > 1e-5
        }
        for i in range(1, len(vertices)):
            max_flow, partition = ford_fulkerson(
                capacities, vertices[0], vertices[i]
            )
            if max_flow <= 1-1e-5:
                p1 = set(partition)
                p2 = set(vertices) - p1
                min_cut = sum(
                    capacities.get((i, j), 0)
                    for i in p1 for j in p2
                )
                cutvarnames = [tuple2var('x', i, j) for i in p1 for j in p2]
                coeffs = [1 for i in range(len(cutvarnames))]
                self.addCut(cutvarnames, coeffs, ampls.CutDirection.GE, 1)
                print('> max-flow: {}, min-cut: {}, must be == 1'.format(
                    max_flow, min_cut))
                return 0
        return 0

    def run(self):
        try:
            if ENABLE_CB_MIPSOL and self.getAMPLSWhere() == ampls.Where.MIPSOL:
                return self.mipsol()
            elif ENABLE_CB_MIPNODE and self.getAMPLSWhere() == ampls.Where.MIPNODE:
                return self.mipnode()
            else:
                return 0
        except Exception as e:
            print('Error:', e)
            return 1


if ENABLE_CALLBACK:
    cb = MyCallback()
    m.setCallback(cb)

m.optimize()
obj = m.getObj()
nvars = m.getNumVars()
print("Solved for {} variables, objective {}".format(nvars, obj))
if m.getStatus() == ampls.Status.OPTIMAL:
    ampl.importSolution(m)
    ampl.display('total')
