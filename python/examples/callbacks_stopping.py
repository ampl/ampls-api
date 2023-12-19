#!/usr/bin/env python
# -*- coding: utf-8 -*-
from amplpy import AMPL

import amplpy_gurobi as ampls
SOLVER = "gurobi"

# Example description
# Stopping the solution process given a certain criteria
# using callbacks

## Desired MIP gap where to stop execution
DESIREDGAP = 0.1
MUTE = True


class StoppingCallback(ampls.GenericCallback):
    '''User defined class implementing the stopping functionality.
       Returning -1 from a callback makes the solver interrupt the
       solution process'''
    def __init__(self):
        self.last_bound = 10e10

    def _evaluate_objs(self, mipsol : bool):
        obj_bound = self.get_value(ampls.Value.MIP_OBJBOUND)
        gap = self.get_value(ampls.Value.MIP_RELATIVEGAP)
        if not mipsol: # print only if bound has improved
            if self.last_bound == obj_bound: return 0
            self.last_bound = obj_bound;
        nnz =  sum(x != 0 for x in self.get_solution_vector())
        print(f"Current objective: {self.getObj()} - bound: {obj_bound} - relgap: {gap} - nonzeroes: {nnz}")
        if gap < DESIREDGAP:
            print("Desired gap reached, terminating")
            return -1
        return 0

    def run(self):
        t = self.get_ampl_where()
        if t == ampls.Where.MIPNODE:
            if not MUTE: print(f"New MIP node, count {self.get_value(ampls.Value.MIP_NODES)}")
            return self._evaluate_objs(False)
        if t == ampls.Where.MIPSOL:
            if not MUTE: print(f"New MIP solution, count {self.get_value(ampls.Value.MIP_NODES)}")
            return self._evaluate_objs(False)
        return 0


# Load a model with amplpy
ampl = AMPL()
ampl.read("models/queens.mod")

# Export to ampls
model = ampl.to_ampls(SOLVER);
# Specify option to return the mip gap to AMPL after solving,
# see https://mp.ampl.com/features-guide.html, useful if
# the MIP gap value is then needed in amplpy
model.setOption("mip:return_gap", 7)

# Declare a callback
cb = StoppingCallback()

# In case of CPLEX, multithreading when using callbacks must
# be handled by the user. See cplex-genericbranch
if SOLVER == "cplex": model.set_option("threads", 1)

# Set the callback and start the optimization
model.set_callback(cb)
model.optimize()

# When the optimization finishes, show the results
obj = model.get_obj()
if abs(158 - obj) >= DESIREDGAP:
    print("Error")

# Import the solution back to amplpy
ampl.import_solution(model)
ampl_obj = ampl.get_current_objective().value()
if abs(158 - ampl_obj) >= DESIREDGAP:
    print("Error")

# Print MIP gap
ampl_rel_gap = ampl.get_value("Initial.relmipgap")
print(f"From AMPL we see a relative gap of {ampl_rel_gap}")
