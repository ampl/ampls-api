#!/usr/bin/env python
# -*- coding: utf-8 -*-
from ast import Try
import unittest
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from test_base import TestBase
from amplpy import AMPL

import amplpy_cplex as ampls
SOLVER = "cplex"

# Show how to set options, both at loading time and afterwards

class TestOptions(TestBase):
    @staticmethod
    def create_model():
        ampl = AMPL()
        ampl.eval("""param pi := 4 * atan(1);
                     var x >= -4 * pi, <= -pi;
                     var y >= -1, <= 1;
                     s.t.Sin01: y <= sin(x);
                     maximize SumXY : x + y;""")
        return ampl

    def test_get_and_set_options(self):
        ampl = TestOptions.create_model()

        # Set converter option at load time
        model = ampl.to_ampls(SOLVER, ["cvt:pre:all=1"])
        assert 1==model.get_int_option("cvt:pre:all")

        # Try setting a wrong option
        with self.assertRaises(ampls.AMPLSolverException):
            model.set_option("wrongname", 0)

        # Set converter option at load time
        model = ampl.to_ampls(SOLVER, ["cvt:pre:all=0"])
        assert 0==model.get_int_option("cvt:pre:all")

        # Get and set "normal" options
        model.set_option("outlev", 1);
        assert(1 == model.get_int_option("outlev"));

        model.set_option("mip:gap", 0.13);
        assert(0.13 == model.get_double_option("mip:gap"));

        model.set_option("tech:logfile", "mylog");
        assert "mylog" == model.get_string_option("tech:logfile")


    def test_delta(self):
        ampl = TestOptions.create_model()
        options = ["outlev=1", "timelim=10"]
        model = ampl.to_ampls(SOLVER, options)
        if SOLVER == "gurobi":
            drv=model.get_double_option('timelim')
            grbtl=model.getDoubleParam("TimeLimit")
            assert  drv== 10, "Gurobi Model time limit should be 10 but instead is " + str(drv)
            assert  grbtl== 10, "Gurobi Model time limit should be 10 but instead is " + str(grbtl)

    def test_converter_options(self):
        ampl = TestOptions.create_model()

        # Try wrong option name at loading time
        with self.assertRaises(RuntimeError):
                options=["wrong=45"]
                model = ampl.to_ampls(SOLVER, options)

        model = ampl.to_ampls(SOLVER)

        # print the first 5 available options
        for o in model.getOptions()[:5]:
            print(o)

        # Try setting converter option after to_ampls will fail
        with self.assertRaises(ampls.AMPLSolverException):
            model.set_option("cvt:pre:all", 0)


        if SOLVER=="gurobi": # test with an option only supported by gurobi
            # Set gurobi to not to use its native sin support
            # model is reformulated by MP
            OPT = "acc:sin"
            model = ampl.to_ampls(SOLVER, ["acc:sin=0"])
            model.optimize()
            print(f"{OPT} is {model.get_int_option(OPT)}")
            assert model.getIntOption(OPT)==0
            ampl.import_solution(model)

            # Set gurobi to use its native sin support
            model = ampl.to_ampls(SOLVER, ["acc:sin=1"])
            model.optimize()
            print(f"{OPT} is {model.get_int_option(OPT)}")
            assert model.get_int_option(OPT)==1
            ampl.import_solution(model)





if __name__ == "__main__":
    unittest.main()
