import sys
import amplpy_gurobi as grb
from tsp_helpers import tsp_model

# Use mostly gurobi C interface to monitor the solution of a model


class MyCallback(grb.GurobiCallback):
    def __init(self):
        super.__init__(self)

    def log(self, item):
        # self._logfile.write(item)
        print(item)

    def run(self):
        try:
            # Get the data needed to use gurobi C functions related to callbacks
            where = self.where()
            cbdata = self.getCBData()

            if where == grb.GRB_CB_POLLING:
                # Ignore polling callback
                pass
            elif where == grb.GRB_CB_PRESOLVE:
                # Presolve callback
                cdels = self.getInt(grb.GRB_CB_PRE_COLDEL)
                rdels = self.getInt(grb.GRB_CB_PRE_ROWDEL)
                if cdels or rdels:
                    print('%d columns and %d rows are removed' %
                          (cdels, rdels))
            elif where == grb.GRB_CB_SIMPLEX:
                # Simplex callback
                itcnt = self.getInt(grb.GRB_CB_SPX_ITRCNT)
                if itcnt - model._lastiter >= 100:
                    self._lastiter = itcnt
                    obj = self.getDouble(grb.GRB_CB_SPX_OBJVAL)
                    ispert = self.getInt(grb.GRB_CB_SPX_ISPERT)
                    pinf = self.getDouble(grb.GRB_CB_SPX_SPX_PRIMINF)
                    dinf = self.getDouble(grb.GRB_CB_SPX_SPX_DUALINF)
                    if ispert == 0:
                        ch = ' '
                    elif ispert == 1:
                        ch = 'S'
                    else:
                        ch = 'P'
                    print('%d %g%s %g %g' % (int(itcnt), obj, ch, pinf, dinf))
            elif where == grb.GRB_CB_MIP:
                # General MIP callback
                nodecnt = self.getInt(grb.GRB_CB_MIP_NODCNT)
                objbst = self.getDouble(grb.GRB_CB_MIP_OBJBST)
                objbnd = self.getDouble(grb.GRB_CB_MIP_OBJBND)
                solcnt = self.getInt(grb.GRB_CB_MIP_SOLCNT)
                if nodecnt - self._lastnode >= 100:
                    self._lastnode = nodecnt
                    actnodes = self.getInt(grb.GRB_CB_MIP_NODLFT)
                    itcnt = self.getInt(grb.GRB_CB_MIP_ITRCNT)
                    cutcnt = self.getInt(grb.GRB_CB_MIP_CUTCNT)
                    print('%d %d %d %g %g %d %d' % (nodecnt, actnodes,
                                                    itcnt, objbst, objbnd, solcnt, cutcnt))
                if abs(objbst - objbnd) < 0.1 * (1.0 + abs(objbst)):
                    print('Stop early - 10% gap achieved')
                    self.terminate()
                if nodecnt >= 10000 and solcnt:
                    print('Stop early - 10000 nodes explored')
                    self.terminate()
            elif where == grb.GRB_CB_MIPSOL:
                # MIP solution callback
                nodecnt = self.getInt(grb.GRB_CB_MIPSOL_NODCNT)
                obj = self.getDouble(grb.GRB_CB_MIPSOL_OBJ)
                solcnt = self.getInt(grb.GRB_CB_MIPSOL_SOLCNT)
                x = self.getDoubleArray(grb.GRB_CB_MIPSOL_SOL)
                print('**** New solution at node %d, obj %g, sol %d, '
                      'x[0] = %g ****' % (nodecnt, obj, solcnt, x[0]))
            elif where == grb.GRB_CB_MIPNODE:
                # MIP node callback
                print('**** New node ****')
                if self.getInt(grb.GRB_CB_MIPNODE_STATUS) == GRB.Status.OPTIMAL:
                    d = self.getSolutionVector()
                    self.setSolution(d)
            elif where == grb.GRB_CB_BARRIER:
                # Barrier callback
                itcnt = self.getInt(grb.GRB_CB_BARRIER_ITRCNT)
                primobj = self.getInt(grb.GRB_CB_BARRIER_PRIMOBJ)
                dualobj = self.getInt(grb.GRB_CB_BARRIER_DUALOBJ)
                priminf = self.getInt(grb.GRB_CB_BARRIER_PRIMINF)
                dualinf = self.getInt(grb.GRB_CB_BARRIER_DUALINF)
                cmpl = self.getInt(grb.GRB_CB_BARRIER_COMPL)
                print('%d %g %g %g %g %g' % (itcnt, primobj, dualobj,
                                             priminf, dualinf, cmpl))
            elif where == grb.GRB_CB_MESSAGE:
                # Message callback
                msg = self.getMessage()
                self.log(msg)
            return 0
        except Exception as e:
            print(e)


ampl = tsp_model('tsp_51_1.txt')
model = ampl.exportGurobiModel()

# Get the raw C gurobi pointer
grbmodel = model.getGRBmodel()
# Turn off display and heuristics
grb.GRBsetintattr(grbmodel, grb.GRB_INT_PAR_OUTPUTFLAG, 0)
grb.GRBsetdblattr(grbmodel, grb.GRB_DBL_PAR_HEURISTICS, 0)

# Open log file
logfile = open('cb.log', 'w+')

cb = MyCallback()
# Pass data into my callback function
cb._lastiter = -100000
cb._lastnode = -100000
cb._logfile = logfile
cb._vars = model.getNumVars()

# Solve model and capture solution information
model.setCallback(cb)
model.optimize()

print('')
print('Optimization complete')
if model.getIntParam('SolCount') == 0:
    print('No solution found, optimization status = %d' %
          model.getIntParam('Status'))
else:
    print('Solution found, objective = %g' % model.getObj())
    sol = model.getSolutionVector()
    map = model.getVarMap()
    vmap = {v: k for k, v in map.iteritems()}
    for i in range(1, len(sol)):
        if sol[i] != 0:
            print('%s %g' % (vmap[i], sol[i]))

# Close log file
logfile.close()
