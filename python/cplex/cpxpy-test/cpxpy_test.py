from amplpy import AMPL
import cpxpy_c as cpxpy

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
  return name + '['+ ','.join(toAMPLIndex(x) for x in index) + ']'
  
CALL_COUNT_MIP = 0
CALL_COUNT_MIPSOL = 0

class MyCallback(cpxpy.CPLEXCallback):
    def run(self, model, cbdata):
      print("Hello!")
       # global CALL_COUNT_MIP, CALL_COUNT_MIPSOL
       # if where == gpy.GRB_CB_MIPSOL:
       #   CALL_COUNT_MIPSOL += 1
       #   print(self.getSolution())
       #   print("GRB_CB_MIP_SOL #{}!".format(CALL_COUNT_MIPSOL))
       # elif where == gpy.GRB_CB_MIP:
       #   CALL_COUNT_MIP += 1
       #   print("GRB_CB_MIP #{}!".format(CALL_COUNT_MIP))
       #   print(self.getSolution())
       #   if countMIP >= 10:
       #     return 1
       # return 0

ampl.eval("option auxfiles c;")
ampl.eval("write gmodelint;")
d= cpxpy.CPLEXDrv()
m = d.loadModel("modelint.nl")
nvars = m.getNumVars()
#cb = MyCallback()
#m.setCallback(cb)
obj = m.optimize()
obj = m.getObj()

err = m.writeSol()
ampl.eval("solution modelint.sol;")

o = ampl.getObjective('z')

print("Obj values - AMPL={} Gurobi={}".format(o.value(), obj))