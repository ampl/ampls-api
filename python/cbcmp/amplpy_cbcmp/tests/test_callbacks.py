#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest

try:
    from . import TestBase
    from .tsp_helpers import tsp_model
except:
    import TestBase
    from tsp_helpers import tsp_model
import amplpy_cbcmp as ampls
import os

SOLVER='cbcmp'

MUTE=True

class ProgressCallback(ampls.GenericCallback):
   
    def __init__(self):
        super(ProgressCallback, self).__init__()
        self.n_mip_nodes = 0
        self.calls = { w : 0 for w in ampls.Where}
        self.notMapped=[]

    def run(self):
        t = self.getAMPLWhere()
        self.calls[t]+=1
        if not MUTE: print("AMPLS Phase: {}, from solver: {}".format(t, self.getWhereString()))
        if t == ampls.Where.MSG:
            if not MUTE: print(self.getMessage())
        if t == ampls.Where.LPSOLVE:
            if not MUTE: print(
                "LP solve, {} iterations".format(
                    self.getValue(ampls.Value.ITERATIONS)
                )
            )
            return 0
        if t == ampls.Where.PRESOLVE:
            if not MUTE: print(
                "Presolve, eliminated {} rows and {} columns.".format(
                    self.getValue(ampls.Value.PRE_DELROWS),
                    self.getValue(ampls.Value.PRE_DELCOLS),
                )
            )
            return 0
        if t == ampls.Where.MIPNODE:
            self.n_mip_nodes += 1
            if not MUTE: print("New MIP node, count {}".format(self.n_mip_nodes))
        if t == ampls.Where.MIPSOL:
            if not MUTE: print("MIP Solution = {}".format(self.getObj()))
        if t== ampls.Where.NOTMAPPED:
            self.notMapped.append(self.getWhereString())
        return 0

class ProgressCallbackSnakeCase(ampls.GenericCallback):
    def __init__(self):
        super(ProgressCallbackSnakeCase, self).__init__()
        self.n_mip_nodes = 0
        self.calls = { w : 0 for w in ampls.Where}
        self.not_mapped=[]
    def run(self):
        t = self.get_ampl_where()
        self.calls[t]+=1
        if not MUTE: print("AMPLS Phase: {}, from solver: {}".format(t, self.get_where_string()))
        if t == ampls.Where.MSG:
            if not MUTE: print(self.get_message())
        if t == ampls.Where.LPSOLVE:
            if not MUTE: print(f"LP solve, {self.get_value(ampls.Value.ITERATIONS)} iterations")
            return 0
        if t == ampls.Where.PRESOLVE:
            if not MUTE: print("Presolve, eliminated {} rows and {} columns.".format(
                   self.get_value(ampls.Value.PRE_DELROWS), 
                   self.get_value(ampls.Value.PRE_DELCOLS)))
            return 0
        if t == ampls.Where.MIPNODE:
            self.n_mip_nodes += 1
            if not MUTE: print("New MIP node, count {}".format(self.n_mip_nodes))
        if t == ampls.Where.MIPSOL:
            if not MUTE: print("MIP Solution = {}".format(self.get_obj()))
        if t== ampls.Where.NOTMAPPED:
            self.not_mapped.append(self.getWhereString())
        return 0


class TestCallbacks(TestBase.TestBase):
    def _callback(self, cb):
        ampl = tsp_model(os.path.join(self._data_dir, "tsp_40_1.txt"))
        gm = ampl.to_ampls(SOLVER)
        gm.setCallback(cb)
        gm.optimize()
        obj = gm.getObj()
        self.assertAlmostEqual(376.96, obj, delta=0.01)

    def test_progress_callback(self):
        cb=ProgressCallback()
        self._callback(cb)
        from pprint import pprint
        pprint(cb.calls)

    def test_progress_callback_snake_case(self):
        cb = ProgressCallbackSnakeCase()
        self._callback(cb)
        from pprint import pprint
        pprint(cb.calls)


if __name__ == "__main__":
    unittest.main()
