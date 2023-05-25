#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from test_base import TestBase

"""
This example shows how to create a model using amplpy
Export it to a specific solver
Access its properties/method via the AMPLS generic API
Access its properties via solver-specific methods
Change the import below to change solver
"""

import pandas as pd
from amplpy import AMPL
import amplpy_xpress as ampls

SOLVER = "xpress"


def makeAmplModel(
    numVars=10, flipObjective=False, makeInfeasible=False, presolveLevel: bool = 10
):
    """Create an instance of AMPL and a model"""
    ampl = AMPL()
    # Note that if a model is detected as infeasible by the AMPL presolver,
    # it will not be exported to the solver, and an exception will be thrown.

    ampl.set_option("presolve", presolveLevel)

    ampl.eval(
        """
    set varIndexes within Integers;
    set constraintIndexes within Integers;

    param varCosts { varIndexes };
    param rightHandSides { constraintIndexes };

    var x {i in varIndexes} >=0;

    maximize objectiveFunction:
        sum { i in varIndexes } varCosts[i] * x[i];

    subject to mainConstraints { j in constraintIndexes } :
        x[j] + x[j+1] <= rightHandSides[j];"""
    )

    varIndexDf = pd.DataFrame(
        data=[(i, i if not flipObjective else -i) for i in range(numVars)],
        columns=["varIndexes", "varCosts"],
    ).set_index("varIndexes")
    ampl.setData(varIndexDf, set_name="varIndexes")

    constraintIndexesDf = pd.DataFrame(
        data=[(i, 1 if not makeInfeasible else -1) for i in range(numVars - 1)],
        columns=["constraintIndexes", "rightHandSides"],
    ).set_index("constraintIndexes")
    ampl.setData(constraintIndexesDf, set_name="constraintIndexes")
    return ampl


def solveModel(ampl, options={}, useNativeCall=False):
    model = ampl.to_ampls(SOLVER)
    for o, v in options.items():
        try:
            model.set_option(o, v)  # Set options from
            # ampl driver (see https://dev.ampl.com/solvers/gurobi/options.html)
            # Most of them will work the same way across solvers
        except Exception as e:
            print(f"Exception while setting {o}: {str(e)}")

    if SOLVER == "gurobi":
        # For each solver there are also wrappers for the
        # solver parameters' accessors. See https://www.gurobi.com/documentation/10.0/refman/c_parameter_examples.html
        model.set_param("Threads", 4)
        # A third option is to use the gurobi C API directy, by getting access to the underlying
        # gurobi model/environment pointer. This is suitabe to use functionalities provided
        # by the solver C API
        ampls.GRBsetintparam(model.get_grb_env(), "Threads", 4)
    elif SOLVER == "cplex":
        # Wrapper on native
        model.set_param(ampls.CPXPARAM_Threads, 4)
        # Fully native
        ampls.CPXsetintparam(model.get_cplex_env(), ampls.CPXPARAM_Threads, 4)
    elif SOLVER == "xpress":
        # Wrapper on native
        model.set_param(ampls.XPRS_THREADS, 4)
        # Fully native
        ampls.XPRSsetintcontrol(model.get_xprs_prob(), ampls.XPRS_THREADS, 4)

    # The same applies to any function; the two statements below are therefore equivalent,
    # the first uses the (generic) function call, the latter uses the Gurobi C API directly:
    if useNativeCall:
        if SOLVER == "gurobi":
            ampls.GRBoptimize(model.get_grb_model())
        elif SOLVER == "cplex":
            ampls.CPXmipopt(model.get_cplex_env(), model.get_cplex_lp())
        elif SOLVER == "xpress":
            ampls.XPRSlpoptimize(model.get_xprs_prob())
    else:
        model.optimize()
    ampl.import_solution(model)
    return model


def createAndSolveSimpleModel(numVars: int = 10):
    ampl = makeAmplModel()
    model = solveModel(ampl)

    # Check status using generic interface/enumeration
    print(f"Status is {model.get_status().name}")
    assert model.get_status() == ampls.Status.OPTIMAL

    # Once again, use also solver-specific methods to check the status
    if SOLVER == "gurobi":
        status = model.get_int_attr("Status")
        assert model.get_int_attr("Status") == ampls.GRB_OPTIMAL
    elif SOLVER == "cplex":
        status = ampls.CPXgetstat(model.get_cplex_env(), model.get_cplex_lp())
        assert status == ampls.CPXMIP_OPTIMAL
    elif SOLVER == "xpress":
        status = model.get_int_attr(ampls.XPRS_LPSTATUS)
        assert status == ampls.XPRS_LP_OPTIMAL

    # Num Vars is even -> last var idx is odd -> expect odd idx vars to be 1
    # Num Vars is odd -> last var idx is even -> expect even idx vars to be 1
    expectedSolution = [abs(i % 2 - numVars % 2) for i in range(numVars)]
    expectedObjective = sum([i * expectedSolution[i] for i in range(numVars)])
    solverObjective = model.getObj()
    assert solverObjective == expectedObjective
    # Check that AMPL has received the correct objective
    assert solverObjective == ampl.getCurrentObjective().value()
    print("Completed Simple Model Test.")


def createAndSolveInfeasibleModel(presolveLevel: bool = 10):
    ampl = makeAmplModel(makeInfeasible=True, presolveLevel=presolveLevel)
    # Set some solver options
    # Setting iisfind to 1 to find the Irreducible Infeasibility Subset
    options = {"pre:scale": 3, "outlev": 1, "iisfind": 1}
    model = solveModel(ampl, options)
    print(f"Status is {model.get_status().name}")
    assert model.get_status() == ampls.Status.INFEASIBLE
    # Display IIS for infeasible model by converting it to a pandas dataframe
    print(ampl.get_data("_varname, _var.iis").to_pandas().set_index("_varname"))
    print(ampl.get_data("_conname, _con.iis").to_pandas().set_index("_conname"))
    print("Completed Infeasible Model Test.")


class TestMultipleModels(TestBase):
    def test_multiple_models(self):
        try:
            # With presolve set to a high level, AMPL detects the infeasibilitty
            # and does not export the model. Throws an explainatory runtime error
            # createAndSolveInfeasibleModel(10)
            pass
        except RuntimeError as e:
            print(e)
        # Turning off presolve makes ampl actually export the model
        # In the fucntion we'll set some options to find the source of the infeasibility
        createAndSolveInfeasibleModel(0)

        # Repeated solves below for feasible model:
        for i in range(0, 10):
            with self.subTest(i=i):
                createAndSolveSimpleModel()

    def test_repeated_exports(self):
        ampl = makeAmplModel()
        for i in range(0, 10):
            with self.subTest(i=i):
                model = solveModel(ampl)
                assert model.get_status() == ampls.Status.OPTIMAL


if __name__ == "__main__":
    unittest.main()
