#!/usr/bin/env python
# -*- coding: utf-8 -*-
from amplpy import AMPL

import amplpy_cplex as ampls
SOLVER = "cplex"

# Example description
# How to set options in the solver driver using:
#   1) solver driver options 
#   2) solver specific parameters

def define_model():
    ampl = AMPL()
    ampl.eval('var x binary; var y binary; var z binary;'
              'minimize TotalSum: z + 1;'
              'subj to C1: x+y >= 1;')
    return ampl

def solve(ampl: AMPL):
    mod = ampl.to_ampls(SOLVER, ["sol:stub=test_multi"])

    # Use AMPL driver options
    # See https://mp.ampl.com/features-guide.html
    # and https://dev.ampl.com/solvers/index.html
    mod.set_option("outlev", 1)

    # Use solver specific parameters
    if SOLVER=="gurobi":
        mod.set_param(ampls.GRB_INT_PAR_SOLUTIONLIMIT, 5)
    if SOLVER=="cplex":
        mod.set_param(ampls.CPXPARAM_MIP_Pool_Capacity, 5)
    if SOLVER=="xpress":
        mod.set_param(ampls.XPRS_MSP_SOLUTIONS, 5)

    # Optimize
    mod.optimize()

    # Import into AMPL
    ampl.import_solution(mod)
    print(f"Found {ampl.get_value('Initial.nsol')} solutions")
    print(f"Objective value: {mod.get_obj()}")


if __name__ == "__main__":
    a = define_model()
    solve(a)