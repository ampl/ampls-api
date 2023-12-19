#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from test_base import TestBase
from tsp_helpers import tsp_model

from amplpy import AMPL
import amplpy_gurobi as ampls
import os

SOLVER = "gurobi"

MUTE = True

## Add mipgap monitoring and/or stopping criteria
DESIREDGAP = 0.1


def load_model(modelfile : str) -> AMPL:
    a = AMPL()
    a.read(modelfile)
    return a

def _evaluate_objs(self, mipsol : bool):
         obj_bound = self.getValue(ampls.Value.MIP_OBJBOUND)
         gap = self.getValue(ampls.Value.MIP_RELATIVEGAP)
         if not mipsol: # print only if bound has improved
            if self.last_bound == obj_bound: return 0
         self.last_bound = obj_bound;
         print(f"Current objective: {self.getObj()} - bound: {obj_bound} - relgap: {gap}")
         if gap < DESIREDGAP:
             print("Desired gap reached, terminating")
             return -1
         return 0

class ProgressCallback(ampls.GenericCallback):
    def __init__(self):
        super(ProgressCallback, self).__init__()
        self.calls = {w: 0 for w in ampls.Where}
        self.not_mapped = []
        ProgressCallback._evaluateObjs = _evaluate_objs
        self.last_bound = 10e10
    

    def run(self):
        t = self.getAMPLWhere()
        self.calls[t] += 1
        if not MUTE:
            print("AMPLS Phase: {}, from solver: {}".format(t, self.getWhereString()))
        if t == ampls.Where.MSG:
            if not MUTE:
                print(self.getMessage())
        if t == ampls.Where.LPSOLVE:
            if not MUTE:
                print("LP solve, {} iterations".format(self.getValue(ampls.Value.ITERATIONS)))
            return 0
        if t == ampls.Where.PRESOLVE:
            if not MUTE:
                print(
                    "Presolve, eliminated {} rows and {} columns.".format(
                        self.getValue(ampls.Value.PRE_DELROWS),
                        self.getValue(ampls.Value.PRE_DELCOLS)
                    )
                )
            return 0
        if t == ampls.Where.MIPNODE:
            if not MUTE:
                 print(f"Num nodes: {self.getValue(ampls.Value.MIP_NODES)}")
            return self._evaluateObjs(False)
        if t == ampls.Where.MIPSOL:
            return self._evaluateObjs(True)
        if t == ampls.Where.NOTMAPPED:
            self.not_mapped.append(self.getWhereString())
        return 0


class ProgressCallbackSnakeCase(ampls.GenericCallback):
    def __init__(self):
        super(ProgressCallbackSnakeCase, self).__init__()
        self.calls = {w: 0 for w in ampls.Where}
        self.not_mapped = []
        self.last_bound = 10e10
        ProgressCallbackSnakeCase._evaluate_objs = _evaluate_objs

    def run(self):
        t = self.get_ampl_where()
        self.calls[t] += 1
        if not MUTE:
            print("AMPLS Phase: {}, from solver: {}".format(t, self.get_where_string()))
        if t == ampls.Where.MSG:
            if not MUTE:
                print(self.get_message())
        if t == ampls.Where.LPSOLVE:
            if not MUTE:
                print(f"LP solve, {self.get_value(ampls.Value.ITERATIONS)} iterations")
            return 0
        if t == ampls.Where.PRESOLVE:
            if not MUTE:
                print(
                    "Presolve, eliminated {} rows and {} columns.".format(
                        self.get_value(ampls.Value.PRE_DELROWS),
                        self.get_value(ampls.Value.PRE_DELCOLS),
                    )
                )
            return 0
        if t == ampls.Where.MIPNODE:
            if not MUTE:
                print(f"New MIP node, count {self.get_value(ampls.Value.MIP_NODES)}")
            return self._evaluate_objs(False)
        if t == ampls.Where.MIPSOL:
            return self._evaluate_objs(False)
        if t == ampls.Where.NOTMAPPED:
            self.not_mapped.append(self.getWhereString())
        return 0

class TestCallbacks(TestBase):
    def test_progress_callback(self):
        from pprint import pprint

        cb = ProgressCallback()
        ampl = load_model(os.path.join(self._data_dir, "queens.mod"))
        model = ampl.to_ampls(SOLVER)
        cb.model=model
        if SOLVER == "cplex": model.set_option("threads", 1)
        model.setCallback(cb)
        model.optimize()
        obj = model.getObj()
        self.assertAlmostEqual(158, obj, delta=DESIREDGAP)
        ampl.importSolution(model)
        ampl_obj = ampl.getCurrentObjective().value()
        self.assertAlmostEqual(158, ampl_obj, delta=DESIREDGAP)
        pprint(cb.calls)

    def test_progress_callback_snake_case(self):
        from pprint import pprint

        cb = ProgressCallbackSnakeCase()
        ampl = load_model(os.path.join(self._data_dir, "queens.mod"))
        model = ampl.to_ampls(SOLVER)
        cb.model=model
        if SOLVER == "cplex": model.set_option("threads", 1)
        model.set_callback(cb)
        model.optimize()
        obj = model.get_obj()
        self.assertAlmostEqual(158, obj, delta=DESIREDGAP)
        ampl.import_solution(model)
        ampl_obj = ampl.get_current_objective().value()
        self.assertAlmostEqual(158, ampl_obj, delta=DESIREDGAP)

        pprint(cb.calls)


if __name__ == "__main__":
    unittest.main()
