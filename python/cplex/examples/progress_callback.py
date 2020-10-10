import sys
import amplpy_cplex_swig as ampls

# Define my callback function (generic)
class MyCallback(ampls.GenericCallback):
    def run(self):
          t = self.getWhere()
          print(t)
          if t ==ampls.Where.MSG:
            #print(self.getMessage())
            print("msg")
          if t ==ampls.Where.LPSOLVE:
            print("During LP solve")
          if t==ampls.Where.PRESOLVE:
            print("Presolve")
          if t==ampls.Where.MIPSOL:
            print("MIP Solution = {}".format(self.getObj()))
          return 0

class MyCPLEXCallback(ampls.CPLEXCallback):
    def run(self):
          print(dir(self))
          w = self.getWhere()
          #where = self.getWhereText()
          print(w)
          return 0
          
# Read model from file
d = ampls.CPLEXDrv()
mn = "D:\\Development\\AMPL\\solvers-public\\test\\models\\tsp.nl"
model = d.loadModel(mn)
cb = MyCallback ()

# Solve model and capture solution information
model.setCallback(cb)
model.optimize()
print('Optimization complete')
