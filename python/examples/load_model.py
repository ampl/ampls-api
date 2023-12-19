#!/usr/bin/env python
# -*- coding: utf-8 -*-
from amplpy import AMPL

import amplpy_gurobi as ampls 
SOLVER = "gurobi"

# Example description
# Shows how to export a model from amplpy to ampls,
# how to solve it and how to get the solution as a vector or
# as a dictionary

def doStuff(a):
    '''Generic function doing the optimization and reading the results'''

    # Optimize with default settings
    a.optimize()
    print("Model status:", a.get_status())
    # Print the objective function
    print("Objective:", a.get_obj())
    
    # Get the solution as vector and count the nonzeroes
    sol = a.get_solution_vector()
    countnz = sum(x != 0 for x in sol)

    # Get the solution as dictionary (name : value)
    sol = a.get_solution_dict()
    nonzeroes = {name : value for name, value in sol.items() if value != 0}
    for (name, value) in nonzeroes.items():
        print("{} = {}".format(name, value))

    print(f"Non zeroes: vector = {countnz}, dict = {len(nonzeroes)}")

# Using amplpy
ampl = AMPL()
ampl.read("models/queens.mod")
ampl.param["size"]=10
# Export to specified solver
ampls_model = ampl.to_ampls(SOLVER)
# Call generic function
doStuff(ampls_model)