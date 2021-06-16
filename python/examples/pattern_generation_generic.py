# Import utilities
import os, sys
from math import floor, ceil
from amplpy import AMPL

#import amplpy_gurobi as ampls
import amplpy_cplex as ampls
from patch import  *

# For notebooks
#from __future__ import print_function
#from amplpy import register_magics
##register_magics('_ampl_cells')  # Store %%ampl cells in the list _ampl_cells



solver = "cplex"

master="""param nPatterns integer > 0;

set PATTERNS = 1..nPatterns;  # patterns
set WIDTHS;                   # finished widths
param order {WIDTHS} >= 0;    # rolls of width j ordered
param overrun;                # permitted overrun on any width
param rawWidth;               # width of raw rolls to be cut
param rolls {WIDTHS,PATTERNS} >= 0, default 0;   
                              # rolls of width i in pattern j
var Cut {PATTERNS} integer >= 0;  # raw rolls to cut in each pattern

minimize TotalRawRolls: sum {p in PATTERNS} Cut[p];
subject to OrderLimits {w in WIDTHS}:
  order[w] <= sum {p in PATTERNS} rolls[w,p] * Cut[p] <= order[w] + overrun;"""

subproblem = """set SIZES;          # ordered widths
param cap >= 0;     # width of raw rolls 
param val {SIZES};  # shadow prices of ordered widths (from Master)
var Qty {SIZES} integer >= 0;  # number of each width in generated pattern
maximize TotVal: sum {s in SIZES} val[s] * Qty[s];
subject to Cap: sum {s in SIZES} s * Qty[s] <= cap;"""


casename = 'UserCut3'

roll_width = 6000
overrun = 3

orders = {
1630: 172, 1625: 714,  1620: 110, 1617: 262, 1540: 32, 1529: 100, 1528: 76,
1505: 110, 1504: 20, 1484: 58, 1466: 15, 1450: 10, 1283: 40, 1017: 50,
970: 70, 930: 8, 916: 210, 898: 395, 894: 49, 881: 17, 855: 20, 844: 10,
805: 718, 787: 17, 786: 710, 780: 150, 754: 34, 746: 15, 707: 122, 698: 7,
651: 10, 644: 15, 638: 10, 605: 10, 477: 4, 473: 34, 471: 25, 468: 10,
460: 908, 458: 161, 453: 765, 447: 21, 441: 20, 422: 318, 421: 22,
419: 382, 396: 22,  309: 123,266: 35
}

widths = list(sorted(orders.keys(), reverse=True))

Master = AMPL()
#Master.eval(_ampl_cells[0])
Master.eval(master)

# Send scalar values
Master.param['nPatterns'] = len(widths)
Master.param['overrun'] = overrun
Master.param['rawWidth'] = roll_width

# Send order vector
Master.set['WIDTHS'] = widths
Master.param['order'] = orders

# Generate and send initial pattern matrix
Master.param['rolls'] = {
    (widths[i], 1+i): int(floor(roll_width/widths[i]))
    for i in range(len(widths))
}

Sub = AMPL()
#Sub.eval(_ampl_cells[1])
Sub.eval(subproblem)

Sub.set['SIZES'] = widths
Sub.param['cap'] = roll_width

# Define a param for sending new patterns to master problem
Master.eval('param newPat {WIDTHS} integer >= 0;')

# Set solve options
Master.option['solver'] = solver
Master.option['relax_integrality'] =  1

Sub.option['solver'] = solver


while True:
    Master.solve()

    Sub.param['val'].setValues(Master.con['OrderLimits'].getValues())
    Sub.solve()
    if Sub.obj['TotVal'].value() <= 1.00001:
        break

    Master.param['newPat'].setValues(Sub.var['Qty'].getValues())
    Master.eval('let nPatterns := nPatterns + 1;')
    Master.eval('let {w in WIDTHS} rolls[w, nPatterns] := newPat[w];')

stopdict = { 'time'   : (  5,    15,   30 ),
             'gaptol' : ( .0002, .002, .02 )
           }

class MyCallback(ampls.GenericCallback):
    def setCurrentGap(self):
      print("Increasing gap tolerance to %.2f%%" % \
                    (100*self._stoprule['gaptol'][self._current]))
      ampls_model.setAMPLsParameter(ampls.SolverParams.DBL_MIPGap,
                             self._stoprule['gaptol'][self._current])
    def run(self):
        where = self.getAMPLWhere()
        if where == ampls.Where.MIPNODE:
            runtime = self.getValue(ampls.Value.RUNTIME).dbl
            if runtime >= self._stoprule['time'][self._current]:
                print(f"Stopping optimization at {runtime} seconds")
                self._continueOpt = True
                self._current += 1
                return 1
        return 0

Master.option['relax_integrality'] = 0
# Export model to ampls
# If we plan to import the results back to AMPL, we have to explicitly set what additional
# suffixes we want returned at this stage
# In this case, we want to return the mip gap as a suffix
ampls_model = Master.exportModel(solver, ["return_mipgap=5"])

callback = MyCallback()
ampls_model.setCallback(callback)

# Convey the stopping rule to the callback
if len(stopdict) == 0:
    callback._stoprule = {'time': (1e+10,), 'gaptol': (1,)}
else:
    stopdict['time'] += (1e+10,)
    stopdict['gaptol'] += (1,)
    callback._stoprule = stopdict
callback._current = 0

# Invoke solver
# Most solvers (e.g. Gurobi https://support.gurobi.com/hc/en-us/articles/360039233191-How-do-I-change-parameters-in-a-callback-)
# do not support changing optimization parameters from a callback
# Instead we have to stop the optimization, change the desired parameters
# and start it again
callback._continueOpt = True
while callback._continueOpt:
  callback._continueOpt = False
  ampls_model.optimize()
  if callback._continueOpt:
    callback.setCurrentGap()


# Import solution from the solver
Master.importSolution(ampls_model)

if solver == "gurobi":
  print(f"From Gurobi MIPGap={100*ampls_model.getDoubleAttr(GRB_DBL_ATTR_MIPGAP):.3f}%")

print(f"From AMPL MIPGap={100*Master.getValue('TotalRawRolls.relmipgap'):.3f}%")
print(f"Objective value: {Master.getValue('TotalRawRolls')}")