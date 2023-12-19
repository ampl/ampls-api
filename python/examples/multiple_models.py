import amplpy
import pandas as pd
from typing import Any, Dict

import amplpy_cplex as ampls
SOLVER = "cplex"

# Example description
# Shows how to create multiple models with AMPL, export an AMPLs instance
# for a specific solver, set some solver options and get status/solution, and get the results
# back in AMPL. It also shows how to use the native C API of various solvers


def makeAmplModel(num_vars :int=10, flip_objective:bool = False, infeasible:bool = False, 
                  presolve_level : bool = 10):
    """ Create an instance of AMPL and a model"""
    ampl = amplpy.AMPL()
    # Note that if a model is detected as infeasible by the AMPL presolver,
    # it will not be exported to the solver, and an exception will be thrown.
    ampl.set_option("presolve", presolve_level)
    
    ampl.eval("""
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
        data = [(i,i if not flip_objective else -i) for i in range(num_vars)],
        columns = ["varIndexes","varCosts"]
    ).set_index("varIndexes")
    ampl.setData(varIndexDf,set_name = "varIndexes")
    
    constraintIndexesDf = pd.DataFrame(
        data = [(i,1 if not infeasible else -1) for i in range(num_vars-1)],
        columns = ["constraintIndexes","rightHandSides"]
    ).set_index("constraintIndexes")
    ampl.setData(constraintIndexesDf,set_name = "constraintIndexes")
    return ampl

def solveModel(ampl:amplpy.AMPL, options:Dict[str,Any] = {}, useNativeCall: bool=True):
    """Solves a model by exporting it to ampls and using either the generic optimize()
       function or the solver native C API"""
    model= ampl.to_ampls(SOLVER)
    for o, v in options.items():
        model.set_option(o,v) # Set options from
       # ampl driver (see for example https://dev.ampl.com/solvers/gurobi/options.html,  
       #  https://dev.ampl.com/solvers/xpress/options.html# )
       # Most of them will work the same way across solvers
    
    # For specific solvers there are also wrappers for the 
    # gurobi parameter accessors. See https://www.gurobi.com/documentation/10.0/refman/c_parameter_examples.html
    if SOLVER == "gurobi":
        model.set_param("Threads", 4) 

    # A third option is to use the solver C API directy, by getting access to the underlying 
    # gurobi model/environment pointer. This is suitabe to use functionalities provided
    # by the specific solver C API
    # The two statmeents below are therefore equivalent, the first uses the Gurobi C API directly,
    # the latter uses the (generic) function provided by ampls
    if useNativeCall:
        if SOLVER == "gurobi":
            grbnative = model.get_grb_model()
            ampls.GRBoptimize(grbnative)
        if SOLVER == "cplex":
            cplexnativeenv = model.get_cplex_env()
            cplexnativemodel = model.get_cplex_lp()
            ampls.CPXlpopt(cplexnativeenv, cplexnativemodel)
        if SOLVER == "xpress":
            xpressnative = model.getXPRSprob()
            ampls.XPRSlpoptimize(xpressnative, None)
    else:
        model.optimize()
    ampl.import_solution(model)
    return model



def createAndSolveSimpleModel(useNativeCall=True):
    num_vars = 10
    ampl = makeAmplModel(num_vars=num_vars)
    model = solveModel(ampl, useNativeCall=useNativeCall)
    
    print(f"Status is {model.get_status().name}")
    assert model.get_status() == ampls.Status.OPTIMAL

    # Num Vars is even -> last var idx is odd -> expect odd idx vars to be 1
    # Num Vars is odd -> last var idx is even -> expect even idx vars to be 1
    expectedSolution = [abs(i%2-num_vars %2) for i in range(num_vars )]    
    expectedObjective = sum( [i*expectedSolution[i] for i in range(num_vars )])    
    solverObjective = model.getObj()
    assert solverObjective == expectedObjective
    # Check that AMPL has received the correct objective
    assert solverObjective == ampl.getCurrentObjective().value()
    print("Completed Simple Model Test.")

def createAndSolveInfeasibleModel(presolve_level : bool = 10, useNativeCall=True):
    ampl = makeAmplModel(infeasible=True, presolve_level=presolve_level)
    # Set some solver options
    # Setting iisfind to 1 to find the Irreducible Infeasibility Subset
    model = solveModel(ampl, {"outlev" : 1, "iisfind" : 1}, useNativeCall=useNativeCall)
    print(f"Status is {model.get_status().name}")
    assert model.get_status() == ampls.Status.INFEASIBLE
    
    # Display IIS for infeasible model by converting it to a pandas dataframe
    print(ampl.get_data("_varname, _var.iis").to_pandas().set_index('_varname'))
    print(ampl.get_data("_conname, _con.iis").to_pandas().set_index('_conname'))
    print("Completed Infeasible Model Test.")


if __name__ == "__main__":
    try:
        # With presolve set to a high level, AMPL detects the infeasibilitty
        # and does not export the model. Throws an explainatory runtime error
        createAndSolveInfeasibleModel(10)
    except ampls.AMPLSolverException as e:
        print(e)
    # Turning off presolve makes ampl actually export the model
    # In the fucntion we'll set some options to find the source of the infeasibility
    createAndSolveInfeasibleModel(0)

    # Repeated solves below for feasible model:
    createAndSolveSimpleModel()
    createAndSolveSimpleModel()
    