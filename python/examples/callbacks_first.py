#!/usr/bin/env python
# -*- coding: utf-8 -*-
from amplpy import AMPL
from pprint import pprint

import amplpy_gurobi as ampls
SOLVER = "gurobi"


# Example description
# Demonstrates the use of generic callbacks

MUTE = True # Set to false to enable more chatter
PRINT_CANDO = False

class Callback(ampls.GenericCallback):
    '''Callback class that implements user defined functionalities'''
    def __init__(self):
        self.calls = {w: 0 for w in ampls.Where}
        self.not_mapped = {}
        self.last_bound = 10e10

    def handle_not_mapped(self):
        '''Function to record names and number of occurences of callback
           calls not mapped by ampls' GenericCallback'''
        where_name = self.get_where_string()
        if where_name in self.not_mapped:
            self.not_mapped[where_name]+=1
        else:
            self.not_mapped[where_name]=1
    
    def run(self):
        '''Function executed at each callback call'''
        # Get the generic stage where the callback has been called
        t = self.get_ampl_where()
        # Record it
        self.calls[t] += 1

        if t == ampls.Where.MSG and not MUTE:
            print(self.get_message())
            return 0

        if t == ampls.Where.LPSOLVE and not MUTE:
            print(f"LP solve, {self.get_value(ampls.Value.ITERATIONS)} iterations")
            return 0

        if t == ampls.Where.PRESOLVE and not MUTE:
            print("Presolve, eliminated {} rows and {} columns.".format(
                        self.get_value(ampls.Value.PRE_DELROWS),
                        self.get_value(ampls.Value.PRE_DELCOLS)))
            return 0

        if t in [ampls.Where.MIPNODE, ampls.Where.MIPSOL]:
            obj_bound = self.get_value(ampls.Value.MIP_OBJBOUND)
            if self.last_bound == obj_bound: return 0 # Don't do anything if bound hasn't improved

            self.last_bound = obj_bound
            nonzeroes = sum(x != 0 for x in self.get_solution_vector())
            gap = self.get_value(ampls.Value.MIP_RELATIVEGAP)
            print(f"{t.name}: nonzeroes={nonzeroes} obj={self.get_obj()} bound={obj_bound} gap={gap}")

        if t == ampls.Where.MIP: 
            if not PRINT_CANDO:
                return 0
            # Certain solvers have stop at points during execution of a MIP
            # solve that are neither a new node or a new solution. Check for functionalities
            if self.can_do(ampls.CanDo.GET_MIP_SOLUTION):
                print("In MIP where I can get MIP solution")
            if self.can_do(ampls.CanDo.GET_LP_SOLUTION):
                print("In MIP where I can get LP solution")
            if self.can_do(ampls.CanDo.IMPORT_SOLUTION):
                print("In MIP where I can import an heuristic solution")

        if t == ampls.Where.NOTMAPPED:
            # In case of not mapped, we record the solver-specific stage
            self.handle_not_mapped()
        return 0


# Load a model with amplpy
ampl = AMPL()
ampl.read("models/queens.mod")
ampl.param["size"]=10

# Export to ampls
model = ampl.to_ampls(SOLVER)

# In case of CPLEX, multithreading when using callbacks must
# be handled by the user. See cplex-genericbranch
if SOLVER == "cplex": model.set_option("threads", 1)

# Create a callback and set it in ampls
cb = Callback()
model.set_callback(cb)

# Start the optimization
model.optimize()

# Import the solution back to amplpy
ampl.import_solution(model)
ampls_obj = model.get_obj()
ampl_obj = ampl.get_current_objective().value()
assert(ampls_obj == ampl_obj)

# Show the number of calls to the callack
pprint(cb.calls)
print(cb.not_mapped)

