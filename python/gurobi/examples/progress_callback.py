import sys
import amplpy_gurobi as gpy

# Define my callback function


class MyCallback(gpy.GRBCallback):

    def log(self, item):
        # self._logfile.write(item)
        print(item)

    def run(self, where):
        if where == gpy.GRB_CB_POLLING:
            # Ignore polling callback
            pass
        elif where == gpy.GRB_CB_PRESOLVE:
            # Presolve callback
            cdels = self.get(gpy.GRB_CB_PRE_COLDEL)
            rdels = self.get(gpy.GRB_CB_PRE_ROWDEL)
            if cdels or rdels:
                print('%d columns and %d rows are removed' % (cdels, rdels))
        elif where == gpy.GRB_CB_SIMPLEX:
            # Simplex callback
            itcnt = self.get(gpy.GRB_CB_SPX_ITRCNT)
            if itcnt - model._lastiter >= 100:
                self._lastiter = itcnt
                obj = self.get(gpy.GRB_CB_SPX_OBJVAL)
                ispert = self.get(gpy.GRB_CB_SPX_ISPERT)
                pinf = self.get(gpy.GRB_CB_SPX_SPX_PRIMINF)
                dinf = self.get(gpy.GRB_CB_SPX_SPX_DUALINF)
                if ispert == 0:
                    ch = ' '
                elif ispert == 1:
                    ch = 'S'
                else:
                    ch = 'P'
                print('%d %g%s %g %g' % (int(itcnt), obj, ch, pinf, dinf))
        elif where == gpy.GRB_CB_MIP:
            # General MIP callback
            nodecnt = self.get(gpy.GRB_CB_MIP_NODCNT)
            objbst = self.get(gpy.GRB_CB_MIP_OBJBST)
            objbnd = self.get(gpy.GRB_CB_MIP_OBJBND)
            solcnt = self.get(gpy.GRB_CB_MIP_SOLCNT)
            if nodecnt - self._lastnode >= 100:
                self._lastnode = nodecnt
                actnodes = self.get(gpy.GRB_CB_MIP_NODLFT)
                itcnt = self.get(gpy.GRB_CB_MIP_ITRCNT)
                cutcnt = self.get(gpy.GRB_CB_MIP_CUTCNT)
                print('%d %d %d %g %g %d %d' % (nodecnt, actnodes,
                                                itcnt, objbst, objbnd, solcnt, cutcnt))
            if abs(objbst - objbnd) < 0.1 * (1.0 + abs(objbst)):
                print('Stop early - 10% gap achieved')
                self.terminate()
            if nodecnt >= 10000 and solcnt:
                print('Stop early - 10000 nodes explored')
                self.terminate()
        elif where == gpy.GRB_CB_MIPSOL:
            # MIP solution callback
            nodecnt = self.get(gpy.GRB_CB_MIPSOL_NODCNT)
            obj = self.get(gpy.GRB_CB_MIPSOL_OBJ)
            solcnt = self.get(gpy.GRB_CB_MIPSOL_SOLCNT)
            x = self.getSolutionVector()
            print('**** New solution at node %d, obj %g, sol %d, '
                  'x[0] = %g ****' % (nodecnt, obj, solcnt, x[0]))
        elif where == gpy.GRB_CB_MIPNODE:
            # MIP node callback
            print('**** New node ****')
            if self.get(gpy.GRB_CB_MIPNODE_STATUS) == GRB.Status.OPTIMAL:
                x = self.getNodeRel(model._vars)
                model.cbSetSolution(model.getVars(), x)
        elif where == gpy.GRB_CB_BARRIER:
            # Barrier callback
            itcnt = self.get(gpy.GRB_CB_BARRIER_ITRCNT)
            primobj = self.get(gpy.GRB_CB_BARRIER_PRIMOBJ)
            dualobj = self.get(gpy.GRB_CB_BARRIER_DUALOBJ)
            priminf = self.get(gpy.GRB_CB_BARRIER_PRIMINF)
            dualinf = self.get(gpy.GRB_CB_BARRIER_DUALINF)
            cmpl = self.get(gpy.GRB_CB_BARRIER_COMPL)
            print('%d %g %g %g %g %g' % (itcnt, primobj, dualobj,
                                         priminf, dualinf, cmpl))
        elif where == gpy.GRB_CB_MESSAGE:
            # Message callback
            msg = self.get(gpy.GRB_CB_MSG_STRING)
            self.log(msg)
        return 0


# Read model from file
model = gpy.DRIVER.loadModel('modelint.nl')

# Turn off display and heuristics
#model.setIntParam('OutputFlag', 0)
#model.setIntParam('Heuristics', 0)

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
