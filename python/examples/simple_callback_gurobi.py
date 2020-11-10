from amplpy import AMPL
import amplpy_gurobi as gpy

ampl = AMPL()

ampl.eval('''
set A:=1..10;
param n{a in A} := a*2;
var x{A} >=0 integer;
maximize z: sum{a in A} x[a]/2;
c{a in A}: x[a] <= n[a];
''')


def toAMPLIndex(index):
    try:
        val = float(index)
        return str(index)
    except ValueError:
        return '\'{}\''.format(index)


def toAMPLName(name, index):
    if index is None:
        return name
    return name + '[' + ','.join(toAMPLIndex(x) for x in index) + ']'


CALL_COUNT_MIP = 0
CALL_COUNT_MIPSOL = 0


class MyCallback(gpy.GurobiCallback):
    def run(self, where):
        global CALL_COUNT_MIP, CALL_COUNT_MIPSOL
        try:
            if where == gpy.GRB_CB_MIPSOL:
                CALL_COUNT_MIPSOL += 1
                print(self.getSolution())
                print("GRB_CB_MIP_SOL #{}!".format(CALL_COUNT_MIPSOL))
            elif where == gpy.GRB_CB_MIP:
                CALL_COUNT_MIP += 1
                print("GRB_CB_MIP #{}!".format(CALL_COUNT_MIP))
                print(self.getSolution())
                if CALL_COUNT_MIP >= 10:
                    return 1
        except Exception as e:
            print(e)
        return 0


m = ampl.exportGurobiModel()
cb = MyCallback()
m.setCallback(cb)
obj = m.optimize()
nvars = m.getNumVars()
err = m.writeSol()

ampl.importSolution(m)
o = ampl.getObjective('z')

print("Obj values - AMPL={} Gurobi={}".format(o.value(), obj))
