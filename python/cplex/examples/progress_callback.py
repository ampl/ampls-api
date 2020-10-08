import sys
import amplpy_cplex_swig as ampls

# Define my callback function


class MyCallback(ampls.GenericCallback):

    def log(self, item):
        # self._logfile.write(item)
        print(item)

    def run(self):
          #print(getMessage())
          t = self.getAMPLType()
          print(t)
          if t ==ampls.Where.msg:
            print(self.getMessage())
          if t ==ampls.Where.lpsolve:
            print("During LP solve")
          if t==ampls.Where.presolve:
            print("Presolve")
          if t==ampls.Where.mipsol:
            print("MIP Solution = {}".format(self.getObjective()))
          return 0

# Read model from file
d = ampls.CPLEXDrv()
mn = "D:\\Development\\AMPL\\solvers-public\\test\\models\\tsp.nl"
model = d.loadModel(mn)

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
#if model.getIntParam('SolCount') == 0:
#    print('No solution found, optimization status = %d' %
#          model.getIntParam('Status'))
#else:
#    print('Solution found, objective = %g' % model.getObj())
#    sol = model.getSolutionVector()
 #   map = model.getVarMap()
 #   vmap = {v: k for k, v in map.iteritems()}
 #   for i in range(1, len(sol)):
 #       if sol[i] != 0:
 #           print('%s %g' % (vmap[i], sol[i]))

# Close log file
logfile.close()
