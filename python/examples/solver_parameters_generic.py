from tsp_helpers import tsp_model

import amplpy_gurobi
import amplpy_cplex
ampls = amplpy_gurobi

statuses = {
    ampls.Status.OPTIMAL: "Optimal",
    ampls.Status.INFEASIBLE: "Infeasible",
    ampls.Status.INTERRUPTED: "Interrupted",
    ampls.Status.LIMIT_ITERATION: "Iterations limit hit",
    ampls.Status.LIMIT_NODE: "Number of nodes limit hit",
    ampls.Status.LIMIT_SOLUTION: "Number of solutions limit hit",
    ampls.Status.LIMIT_TIME: "Time limit hit",
    ampls.Status.UNBOUNDED: "Unbounded",
    ampls.Status.UNKNOWN: "Unknown",
    ampls.Status.NOTMAPPED: "Not mapped"
}

def setParamsAndOptimize(model):
  print(f"\nSolving a {type(model).__name__}")
  # Set relative MIP gap tolerance
  model.setAMPLsParameter(ampls.SolverParams.DBL_MIPGap, 0.01)
  # Set time limit
  model.setAMPLsParameter(ampls.SolverParams.DBL_TimeLimit, 3.0)
  # Set number of solutions limit
  model.setAMPLsParameter(ampls.SolverParams.INT_SolutionLimit, 15)

  # Check the time limit value that we set above
  print(f"Timelimit set to {model.getAMPLsDoubleParameter(ampls.SolverParams.DBL_TimeLimit)}")
  # Optimize
  model.optimize()
  # Display status
  print(f"Objective = {model.getObj()}")
  print("Model status", statuses[model.getStatus()])

# Instantiate the TSP model using amplpy
ampl = tsp_model('tsp_51_1.txt')
# Export a CPLEX model in ampls
cpxmodel = ampl.exportModel("cplex")
# Export a GUROBI model in ampls
grbmodel = ampl.exportModel("gurobi")

# Do the same work on both
for mod in [cpxmodel, grbmodel]:
  setParamsAndOptimize(mod)
