#!/usr/bin/env python
# -*- coding: utf-8 -*-
from math import e
from amplpy import AMPL
import time

import amplpy_gurobi as ampls
SOLVER = "gurobi"

# Example description
# This example shows how to stop the solution process of a MIP 
# problem with different criteria: one based on MIP gap, 
# the other based on the time since an improvement in the solution
# has been achieved.

# Set to true to activate the MIP gap criteria
STOP_AT_MIPGAP = False
# Set to true to stop when no solution improvement has been detected
# for a certain time
STOP_AT_NOSOLUTIONIMPROVEMENT = True

## Desired MIP gap where to stop execution
DESIREDGAP = 0.01
# Define the maximum time without improvement in 
# the solution (in seconds)
MAX_TIME = 10  
 
MUTE = True


class StoppingCallback(ampls.GenericCallback):
    '''User defined class implementing the stopping functionality.
       Returning -1 from a callback makes the solver interrupt the
       solution process'''
    def __init__(self):
        self.last_bound = 10e10
        self.last_obj = None
        self.last_obj_time = None
        
    def _evaluate_mipgap(self, mipsol : bool):
        # At each solution and each node, we check if the desired gap 
        # abs(bestobjective-objbound)/bestobjective
        # has been achieved. If so, terminate the solution process.
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
    
    def _evaluate_solutionprogress(self):
        # Each time we see a new solultion, check its value;
        # If no improvement in the solution value has been detected
        # for a specified amount of time, interrupt the solution process.
        obj = self.get_obj() 
        print(f"Objective value {obj}")
        if obj != self.last_obj:
            if self.last_obj_time is not None:
                print(f"Improvement in the solution after {time.time() - self.last_obj_time} seconds.")
            self.last_obj_time=time.time()
            self.last_obj = obj
        else:
            if time.time() - self.last_obj_time > MAX_TIME:
                print(f"No improvement in the solution for {time.time() - self.last_obj_time} seconds, terminating.")
                return -1
        return 0

    def run(self):
        returnvalue=0
        t = self.get_ampl_where()
        if t == ampls.Where.MIPNODE:
            if STOP_AT_MIPGAP:
                if not MUTE: print(f"New MIP node, count {self.get_value(ampls.Value.MIP_NODES)}")
                return self._evaluate_mipgap(False)
        if t == ampls.Where.MIPSOL:
            print(f"New MIP solution.")
            if STOP_AT_MIPGAP:
                if self._evaluate_mipgap(True)==-1:
                    return -1
            if STOP_AT_NOSOLUTIONIMPROVEMENT:
                return self._evaluate_solutionprogress()
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
if STOP_AT_MIPGAP:
    if abs(158 - ampl_obj) >= DESIREDGAP:
        print("Error")

# Print MIP gap
ampl_rel_gap = ampl.get_value("Initial.relmipgap")
print(f"From AMPL we see a relative gap of {ampl_rel_gap}")
