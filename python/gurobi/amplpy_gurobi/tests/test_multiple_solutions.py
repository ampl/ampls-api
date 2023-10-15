#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from test_base import TestBase

from amplpy import AMPL
import amplpy_gurobi as ampls

SOLVER = "gurobi"


class TestMultipleSolutions(TestBase):
    def test_multiple_solutions(self):
        ampl = AMPL()
        ampl.eval(
            """var x binary; var y binary; var z binary;
        minimize TotalSum: z + 1;
        subj to C1 : x + y >= 1;"""
        )
        model = ampl.to_ampls(SOLVER)
        model.set_option("sol:stub", "stub")
        try:
            model.set_option("presolve", 0)
        except:
            pass
        try:
            model.set_option("sol:poolgap", 0.1)
        except:
            pass
        model.refresh()
        model.optimize()
        ampl.import_solution(model)
        v = int(ampl.get_value("TotalSum.nsol"))
        if SOLVER in ["xpress", "copt", "scip"] :
            self.assertTrue(2 <= v <= 3)
        else:
            self.assertEqual(v, 3)
        print(f"Got {v} solutions")
        for i in range(1, v + 1):
            ampl.import_solution("stub", i)
            self.assertLessEqual(ampl.get_value("TotalSum"), 2.0)
            ampl.display("TotalSum, x, y, z;")


if __name__ == "__main__":
    unittest.main()
