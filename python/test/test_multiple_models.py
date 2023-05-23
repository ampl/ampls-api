#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
try:
    from . import TestBase
except:
    import TestBase

import amplpy
import amplpy_gurobi as ampls
import pandas as pd

SOLVER='gurobi'

def makeAmplModel(numVars=10, flipObjective = False, makeInfeasible = False, 
                  presolveLevel : bool = 10):
    ''' Create an instance of AMPL and a model'''
    ampl = amplpy.AMPL()
    # Note that if a model is detected as infeasible by the AMPL presolver,
    # it will not be exported to the solver, and an exception will be thrown.

    ampl.set_option('presolve', presolveLevel)
    
    ampl.eval('''
    set varIndexes within Integers;
    set constraintIndexes within Integers;

    param varCosts { varIndexes };
    param rightHandSides { constraintIndexes };

    var x {i in varIndexes} >=0;

    maximize objectiveFunction:
        sum { i in varIndexes } varCosts[i] * x[i];

    subject to mainConstraints { j in constraintIndexes } :
        x[j] + x[j+1] <= rightHandSides[j];'''
    )

    varIndexDf = pd.DataFrame(
        data = [(i,i if not flipObjective else -i) for i in range(numVars)],
        columns = ['varIndexes','varCosts']
    ).set_index('varIndexes')
    ampl.setData(varIndexDf,set_name = 'varIndexes')
    
    constraintIndexesDf = pd.DataFrame(
        data = [(i,1 if not makeInfeasible else -1) for i in range(numVars-1)],
        columns = ['constraintIndexes','rightHandSides']
    ).set_index('constraintIndexes')
    ampl.setData(constraintIndexesDf,set_name = 'constraintIndexes')
    return ampl

def solveModel(ampl, options = {}, useNativeCall=False):
    model= ampl.to_ampls(SOLVER)
    for o, v in options.items():
        try:
            model.set_option(o,v) # Set options from
                                   # ampl driver (see https://dev.ampl.com/solvers/gurobi/options.html)
                                   # Most of them will work the same way across solvers
        except Exception as e:
            print(f'Exception while setting {o}: {str(e)}')
    if SOLVER=='gurobi':
        # For specific solvers (e.g. gurobi) there are also wrappers for the 
        # gurobi parameter accessors. See https://www.gurobi.com/documentation/10.0/refman/c_parameter_examples.html
        model.set_param('Threads', 4) 

    # A third option is to use the gurobi C API directy, by getting access to the underlying 
    # gurobi model/environment pointer. This is suitabe to use functionalities provided
    # by the solver C API
    # The two statmeents below are therefore equivalent, the first uses the (generic) function
    # call, the latter uses the Gurobi C API directly:
    if useNativeCall:
        if SOLVER=='gurobi':
            ampls.GRBoptimize(model.get_grb_model())
    else:
        model.optimize()
    ampl.import_solution(model)
    return model


def createAndSolveSimpleModel(numVars: int = 10):
    ampl = makeAmplModel()
    model = solveModel(ampl)
    
    print(f'Status is {model.get_status().name}')
    assert model.get_status() == ampls.Status.OPTIMAL

    # Num Vars is even -> last var idx is odd -> expect odd idx vars to be 1
    # Num Vars is odd -> last var idx is even -> expect even idx vars to be 1
    expectedSolution = [abs(i%2-numVars%2) for i in range(numVars)]    
    expectedObjective = sum( [i*expectedSolution[i] for i in range(numVars)])    
    solverObjective = model.getObj()
    assert solverObjective == expectedObjective
    # Check that AMPL has received the correct objective
    assert solverObjective == ampl.getCurrentObjective().value()
    print('Completed Simple Model Test.')

def createAndSolveInfeasibleModel(presolveLevel : bool = 10):
    ampl = makeAmplModel(makeInfeasible=True, presolveLevel=presolveLevel)
    # Set some solver options
    # Setting iisfind to 1 to find the Irreducible Infeasibility Subset
    options={'pre:scale': 3, 'outlev' : 1}
    if SOLVER=='gurobi': options['iisfind']=1
    model = solveModel(ampl, options)
    print(f'Status is {model.get_status().name}')
    assert model.get_status() == ampls.Status.INFEASIBLE
    
    if SOLVER=='gurobi':
        # Display IIS for infeasible model by converting it to a pandas dataframe
        print(ampl.get_data('_varname, _var.iis').to_pandas().set_index('_varname'))
        print(ampl.get_data('_conname, _con.iis').to_pandas().set_index('_conname'))
    print('Completed Infeasible Model Test.')


class TestMultipleModels(TestBase.TestBase):
    def test_multiple_models(self):
        try:
            # With presolve set to a high level, AMPL detects the infeasibilitty
            # and does not export the model. Throws an explainatory runtime error
            #createAndSolveInfeasibleModel(10)
            pass
        except RuntimeError as e:
            print(e)
        # Turning off presolve makes ampl actually export the model
        # In the fucntion we'll set some options to find the source of the infeasibility
        createAndSolveInfeasibleModel(0)

        # Repeated solves below for feasible model:
        createAndSolveSimpleModel()
        createAndSolveSimpleModel()


if __name__ == '__main__':
    unittest.main()
