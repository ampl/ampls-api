#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
try:
    from . import TestBase
except:
    import TestBase

from amplpy import AMPL
import amplpy_xpress as ampls

def create_model() -> AMPL:
    """ Create an instance of AMPL and a model"""
    ampl = AMPL()
    ampl.eval('''var x binary; var y binary; var z binary;
    minimize TotalSum: z + 1;
    subj to C1 : x + y >= 1;''')
    return ampl

def solve_model(ampl: AMPL):
    model=ampl.to_ampls('xpress')
    model.setOption("sol:stub", "stub");
    model.setOption("sol:poolgap", 0.1);
    model.refresh()
    model.optimize()
    ampl.import_solution(model)
    v = int(ampl.get_value("TotalSum.nsol"))
    assert v==3
    print(f"Got {v} solutions")
    for i in range(1,v+1):
        ampl.import_solution("stub", i)

class TestMultipleSolutions(TestBase.TestBase):
    def test_multiple_solutions(self):
        solve_model(create_model())

if __name__ == "__main__":
    unittest.main()
