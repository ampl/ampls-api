# To add a new cell, type '#%%'
# To add a new markdown cell, type '#%% [markdown]'
#%%
from __future__ import print_function
#from IPython import get_ipython



#%% [markdown]
# # AMPLPY: Pattern Generation
# 
# Documentation: http://amplpy.readthedocs.io
# 
# GitHub Repository: https://github.com/ampl/amplpy
# 
# PyPI Repository: https://pypi.python.org/pypi/amplpy
#%% [markdown]
# ### Imports

#%%

from amplpy import AMPL
import os, sys
from math import floor, ceil
import grbpy_c as gpy


#%%
# Import gurobipy
#sys.path.append('/opt/gurobi810/linux64/lib/python3.6_utf32/')
#sys.path.append('D:/Development/AMPL/escrow-ampls/solvers_dist/build/vs64bin')
#import gurobipy as gpy

#%% [markdown]
# ### Register jupyter magics for AMPL

#%%
#from amplpy import register_magics
#register_magics('_ampl_cells')  # Store %%ampl cells in the list _ampl_cells


#%% [markdown]
#%% [markdown]
# ### Plotting routine

#%%
def cuttingPlot(roll_width, widths, summary, solution):
    import numpy as np
    import matplotlib.pyplot as plt
    nchuncks = max(int(ceil(len(solution)/25.0)), 1)
    chunck_size = int(ceil(len(solution)/float(nchuncks)))
    start = 0
    for i in range(nchuncks):            
        plt.subplot(1, nchuncks, i+1)        
        sol = solution[start:start+chunck_size]
        start += chunck_size
        ind = np.arange(len(sol))
        acc = [0]*len(sol)
        colorlist = ['red','lightblue','orange','lightgreen',
                     'brown','fuchsia','silver','goldenrod']
        for p, (patt, rep) in enumerate(sol):
            for i in range(len(widths)):
                for j in range(patt[i]):
                    vec = [0]*len(sol)
                    vec[p] = widths[i]
                    plt.barh(ind, vec, 0.6, acc, 
                             color=colorlist[i%len(colorlist)], edgecolor='black')
                    acc[p] += widths[i]
        plt.xlim(0, roll_width)
        #plt.xticks(np.arange(0, roll_width, 10))
        plt.yticks(ind, tuple("x {:}".format(rep) for patt, rep in sol))
    plt.suptitle('{}: {} rolls, {} patterns, {:.2f}% waste'.format(
        summary['Data'],
        summary['Obj'],
        len(solution),
        round(100*summary['Waste']/(roll_width*summary['Obj']),2),
    ))   
    plt.show()

#%% [markdown]
# ### Set parameters

#%%
cutdata = 'ChvatalD.py'
stopdata = ''

#%% [markdown]
# ### Read orders, roll_width, overrun; extract widths

#%%
DIR = 'C:/Users/chris/notebooks/todo/CuttingCases'
exec(open(os.path.join(DIR, cutdata)).read(), globals())
widths = list(sorted(orders.keys(), reverse=True))

#%% [markdown]
# ### Set up cutting (master problem) model

#%%
Master = AMPL()
Master.option['ampl_include'] = 'models'
Master.eval('param nPatterns integer > 0;')
Master.eval('set PATTERNS = 1..nPatterns; set WIDTHS; param order {WIDTHS} >= 0; param overrun; param rawWidth;             param rolls {WIDTHS,PATTERNS} >= 0, default 0;   var Cut {PATTERNS} integer >= 0; minimize TotalRawRolls: sum {p in PATTERNS} Cut[p]; OrderLimits {w in WIDTHS}:\n  order[w] <= sum {p in PATTERNS} rolls[w,p] * Cut[p] <= order[w] + overrun;')



#%% [markdown]
# ### Send data to AMPL

#%%
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

#%% [markdown]
# ### Set up for generation loop

#%%
# Define a param for sending AMPL new patterns
Master.eval('param newPat {WIDTHS} integer >= 0;')

# Set solve options
Master.option['solver'] = 'gurobi'
Master.option['relax_integrality'] =  1

#%% [markdown]
# ### Define the knapsack subproblem

#%%
# Define knapsack subproblem
Sub = AMPL()
Sub.option['solver'] = 'gurobi'

Sub.eval('''
    set SIZES;
    param cap >= 0;
    param val {SIZES};
    var Qty {SIZES} integer >= 0;
    maximize TotVal: sum {s in SIZES} val[s] * Qty[s];
    subject to Cap: sum {s in SIZES} s * Qty[s] <= cap;
''')

# Send subproblem data
Sub.set['SIZES'] = widths
Sub.param['cap'] = roll_width

#%% [markdown]
# ### Loop

#%%
while True:
    Master.solve()

    Sub.param['val'].setValues(Master.con['OrderLimits'].getValues())
    Sub.solve()
    if Sub.obj['TotVal'].value() <= 1.00001:
        break

    Master.param['newPat'].setValues(Sub.var['Qty'].getValues())
    Master.eval('let nPatterns := nPatterns + 1;')
    Master.eval('let {w in WIDTHS} rolls[w, nPatterns] := newPat[w];')

#%% [markdown]
# ### Compute and display integer solution

#%%
# Compute integer solution
Master.option['relax_integrality'] = 0

#Master.solve()
#grb_model = Master.exportGurobiModel()
Master.option['auxfiles'] = 'c'
Master.eval('write gpatgen;')

d= gpy.GurobiDrv()
grb_model = d.loadModel('patgen.nl')

class MyCallback(gpy.GRBCallback):
      def run(self, model, cbdata, where):
         """Gurobi callback function."""
         if where == gpy.GRB_CB_MIP:
           runtime = self.getDouble(gpy.GRB_CB_RUNTIME)
           if runtime >= self._stoprule['time'][self._current]:
             print("Reducing gap tolerance to %f at %d seconds" %                 (m._stoprule['gaptol'][m._current], m._stoprule['time'][m._current]))
             self.setDoubleParam("MIPGap", self._stoprule['gaptol'][m._current])
             self._current += 1
         return 0

# Solve with Gurobi API and use a callback
grb_model.setIntParam('LogToConsole', 0)
cb = MyCallback()
grb_model.setCallback(cb)

if len(stopdata) == 0:
    cb._stoprule = {'time': (1e+10,), 'gaptol': (1,)}
else:
    exec(open(stopdata+'.py').read(), globals())
    stopdict['time'] += (1e+10,)
    stopdict['gaptol'] += (1,)
    cb._stoprule = stopdict
cb._current = 0
grb_model.optimize()


grb_model.writeSol()
Master.eval('solution patgen.sol;')
#%% [markdown]
# ### Retrieve solution

#%%
# Prepare summary data
summary = {
    'Data': cutdata,
    'Obj': int(Master.obj['TotalRawRolls'].value()),
    'Waste': Master.getValue(
             'sum {p in PATTERNS} Cut[p] * \
                (rawWidth - sum {w in WIDTHS} w*rolls[w,p])'
             )
}

# Retrieve patterns and solution
npatterns = int(Master.param['nPatterns'].value())
rolls = Master.param['rolls'].getValues().toDict()
cutvec = Master.var['Cut'].getValues().toDict()

print("%d patterns generated" % (npatterns))

# Prepare solution data
solution = [
    ([int(rolls[widths[i], p+1]) 
      for i in range(len(widths))], int(cutvec[p+1]))
    for p in range(npatterns)
    if cutvec[p+1] > 0
][:50]

print("%d patterns used" % (len(solution)))

#%% [markdown]
# ### Display solution

#%%
# Create plot of solution
cuttingPlot(roll_width, widths, summary, solution)


#%%
Master.solve()
Master.option['gurobi_options'] = 'outlev=1 bestbound=1'


#%%
Master.getValue('TotalRawRolls')


#%%
Master.getValue('TotalRawRolls.bestbound')


#%%




