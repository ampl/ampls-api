#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
from . import TestBase
from .tsp_helpers import tsp_model
import amplpy_cplex as ampls
import os


class ProgressCallback(ampls.GenericCallback):
    def __init__(self):
        super(ProgressCallback, self).__init__()
        self.n_mip_nodes = 0

    def run(self):
        t = self.getAMPLSWhere()
        print("AMPL Phase: {}, from solver: {}".format(t, self.getWhereString()))
        if t == ampls.Where.MSG:
            print(self.getMessage())
            return 0
        if t == ampls.Where.LPSOLVE:
            print(
                "LP solve, {} iterations".format(
                    self.getValue(ampls.Value.ITERATIONS).integer
                )
            )
            return 0
        if t == ampls.Where.PRESOLVE:
            print(
                "Presolve, eliminated {} rows and {} columns.".format(
                    self.getValue(ampls.Value.PRE_DELROWS).integer,
                    self.getValue(ampls.Value.PRE_DELCOLS).integer,
                )
            )
            return 0
        if t == ampls.Where.MIPNODE:
            self.n_mip_nodes += 1
            print("New MIP node, count {}".format(self.n_mip_nodes))
        if t == ampls.Where.MIPSOL:
            print("MIP Solution = {}".format(self.getObj()))
        return 0


class ProgressCallbackSnakeCase(ampls.GenericCallback):
    def __init__(self):
        super(ProgressCallbackSnakeCase, self).__init__()
        self.n_mip_nodes = 0

    def run(self):
        t = self.get_ampls_where()
        print("AMPL Phase: {}, from solver: {}".format(t, self.get_where_string()))
        if t == ampls.Where.MSG:
            print(self.get_message())
            return 0
        # if t == ampls.Where.LPSOLVE:
        #     print(
        #         "LP solve, {} iterations".format(
        #             self.get_value(ampls.Value.ITERATIONS).integer
        #         )
        #     )
        #     return 0
        # if t == ampls.Where.PRESOLVE:
        #     print(
        #         "Presolve, eliminated {} rows and {} columns.".format(
        #             self.get_value(ampls.Value.PRE_DELROWS).integer,
        #             self.get_value(ampls.Value.PRE_DELCOLS).integer,
        #         )
        #     )
        #     return 0
        if t == ampls.Where.MIPNODE:
            self.n_mip_nodes += 1
            print("New MIP node, count {}".format(self.n_mip_nodes))
        if t == ampls.Where.MIPSOL:
            print("MIP Solution = {}".format(self.get_obj()))
        return 0


class TestCallbacks(TestBase.TestBase):
    def test_progress_callback(self):
        ampl = tsp_model(os.path.join(self._data_dir, "tsp_10_1.txt"))
        gm = ampl.exportCplexModel()
        cb = ProgressCallback()
        gm.setCallback(cb)
        gm.optimize()
        obj = gm.getObj()
        self.assertAlmostEqual(159.47, obj, delta=0.01)

    def test_progress_callback_snake_case(self):
        ampl = tsp_model(os.path.join(self._data_dir, "tsp_10_1.txt"))
        gm = ampl.exportCplexModel()
        cb = ProgressCallbackSnakeCase()
        gm.set_callback(cb)
        gm.optimize()
        obj = gm.get_obj()
        self.assertAlmostEqual(159.47, obj, delta=0.01)


if __name__ == "__main__":
    unittest.main()
