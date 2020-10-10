import sys
import amplpy_simpleapi_swig as ampls

# Define my generic callback function
class MyCallback(ampls.GenericCallback):
    def __init__(self):
          super(MyCallback, self).__init__()
          self.nMIPnodes = 0
  
    def run(self):
          t = self.getAMPLWhere()
          print("AMPL Phase: {}, from solver: {}".format(t, self.getWhereString()))
          if t == ampls.Where.msg:
           # print(self.getMessage())
            return 0
          if t == ampls.Where.lpsolve:
            print("LP solve, {} iterations".format(self.getValue(ampls.Value.iterations).integer))
            return 0
          if t == ampls.Where.presolve:
            print("Presolve, eliminated {} rows and {} columns.".format(
              self.getValue(ampls.Value.pre_delrows).integer, 
              self.getValue(ampls.Value.pre_delcols).integer))
            return 0
          if t == ampls.Where.mipnode:
            self.nMIPnodes += 1
            print("New MIP node, count {}".format(self.nMIPnodes))
          if t == ampls.Where.mipsol:
            print("MIP Solution = {}".format(self.getObj()))
          return 0



def doStuff(a: ampls.AMPLModel):
  cb = MyCallback()
  a.setCallback(cb)
  a.optimize()
  print(a.getObj())

d = ampls.CPLEXDrv()
cm = d.loadModel("d:\\model.nl")
doStuff(cm)

g = ampls.GurobiDrv()
gm = g.loadModel("d:\\model.nl")
doStuff(gm)