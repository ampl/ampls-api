#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from test_base import TestBase

"""
This example uses Gilmore-Gomory column generation procedure for 
the cutting-stock (roll trim) problem.
Keeps the knapsack problem as an AMPL model, and adds columns
to the cutting-stock via ampls.
Then imports the results in AMPL.

"""
import types
from amplpy import AMPL
import amplpy_copt as ampls

SOLVER = "copt"

WIDTHS = [20, 45, 50, 55, 75]
ORDERS = [48, 35, 24, 10, 8]
RAW_WIDTH = 110

# We represent a pattern as a dictionary width -> numberofcuts
# where we only store the nonzeroes
def generate_default_patterns(widths : list):
    return [{w : RAW_WIDTH // w} for w in widths]
 
def add_patterns(self, patterns : list):
    npat = int(self.param["nPatterns"].get())
    self.param["nPatterns"].set(len(patterns))
    self.eval("update data rolls;")
    self.param['rolls'] = {
        (w, 1+i): n
        for i in range(npat,len(patterns)) 
        for w, n in patterns[i].items()
    }


def cutting_stock_model():
    """Create an instance of AMPL and a model"""
    a = AMPL()
    a.option["presolve"]=0
    a.option["solver"]=SOLVER

    a.eval("""param nPatterns integer >= 0;
    set PATTERNS = 1..nPatterns;  
    set WIDTHS;                 
    param order{ WIDTHS } >= 0;
    param rawWidth;
    param rolls{ WIDTHS,PATTERNS } >= 0, default 0;

    var Cut{ PATTERNS } integer >= 0;

    minimize TotalRawRolls : sum{ p in PATTERNS } Cut[p] + to_come;
    subject to OrderLimits{ w in WIDTHS }:
    sum{ p in PATTERNS } rolls[w, p] * Cut[p]+ to_come >= order[w];""") 

    a.param["nPatterns"]=0
    a.set["WIDTHS"]=WIDTHS
    a.param["order"]=ORDERS
    a.param["rawWidth"]=RAW_WIDTH

    a.add_patterns= types.MethodType( add_patterns, a )
    return a

def generate_pattern(self, duals : list):
    self.param["price"]=duals
    self.solve()
    self.eval("display price;")
    reduced_cost=self.obj['Reduced_Cost'].value()
    if reduced_cost < -0.00001:
        return self.var["Use"].getValues().to_dict()
    return None

def knapsack_model():
    a = AMPL()
    a.option["presolve"]=0
    a.option["solver"]=SOLVER
    a.eval("""set WIDTHS; 
    param rawWidth;
    param price {WIDTHS} default 0.0;
    var Use {WIDTHS} integer >= 0;
    minimize Reduced_Cost: 1 - sum{ i in WIDTHS } price[i] * Use[i];
    subject to Width_Limit: sum{ i in WIDTHS } i * Use[i] <=rawWidth;""")
    a.set["WIDTHS"]=WIDTHS
    a.param["rawWidth"]=RAW_WIDTH

    a.generate_pattern= types.MethodType( generate_pattern, a )
    return a


def run_example():
    # Decalare the two models in AMPL
    cs = cutting_stock_model()
    knap = knapsack_model()

    # Generate starting patterns (each patterns simply cuts the roll
    # all at the same width)
    patterns = generate_default_patterns(WIDTHS)
    # Add them to the AMPL instance
    cs.add_patterns(patterns)
    
    # Export the (relaxed) cutting stock model to ampls
    cs.option["relax_integrality"]=1
    ampls_cs = cs.to_ampls(SOLVER)#, ["outlev=1", "pre:maxrounds=0"])

    

    while True: # Column generation happens in the solver
        # Optimize the cutting stock model, get the dual vector,
        # use it in the knapsack model (in AMPL) to generate 
        # a new pattern 
        ampls_cs.optimize()
        duals = ampls_cs.get_dual_vector()
        print(duals)
        p=knap.generate_pattern(duals)
        
        if p is None: break # No new pattern has been found, finish

        # Keep track of the patterns we add
        patterns.append(p)
        
        # Create the variable indices and coefficients
        index=0
        indices=[]
        coeffs=[]
        for _, c in p.items():
            if c != 0:
                indices.append(index)
                coeffs.append(c)
            index+=1
        # Add variable in the cutting stock model
        ampls_cs.addVariable(indices, coeffs, 0, 10000000,
                                1, ampls.VarType.Continuous)
    
    # Add all the patterns that has been generated in the loop above to
    # the AMPL version of the cutting stock model, then solve the 
    # integer problem to get the final result
    cs.add_patterns(patterns)
    cs.import_solution(ampls_cs)
    cs.option["relax_integrality"]=0
    cs.solve()
    cs.eval("display TotalRawRolls, rolls, Cut;")
    assert cs.get_current_objective().value() == 47

class TestCuttingStock(TestBase):
    def test_cutting_stock(self):
        run_example()


if __name__ == "__main__":
    unittest.main()
