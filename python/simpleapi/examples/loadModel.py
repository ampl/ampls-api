import sys
import amplpy_simpleapi_swig as ampls

#MODEL = "d:\\model.nl"
MODEL = "D:/Development/ampl/solvers-private/build/vs64/bin/nlfileexport.nl"

statuses= {
    ampls.Status.Optimal :"optimal", 
    ampls.Status.Infeasible :"infeasible", 
    ampls.Status.Interrupted :"interrupted", 
    ampls.Status.LimitIteration :"limititeration", 
    ampls.Status.LimitNode :"limitnode", 
    ampls.Status.LimitSolution:"limitsolution",
    ampls.Status.LimitTime:"limittime",
    ampls.Status.Unbounded:"unbounded",
    ampls.Status.Unknown:"unknown",
    ampls.Status.NotMapped:"not mapped"
  }

# Generic function doing the optimization and reading the results
def doStuff(a: ampls.AMPLModel):
  # Optimize with default settings
  a.optimize()
  # Print the status (using the map above to have a description)
  print("Model status ", statuses[a.getStatus()])
  # Print the objective function
  print("Objective: ",a.getObj())
  # Get the solution vector
  d = a.getSolutionVector()
  # Print the non-zeroes, by getting a map
  # Note that this will only work if the col file has been generated
  map = a.getVarMapInverse()
  nonzeroes = [(map[index], d[index]) for index in map if d[index] != 0]
  for (name, value) in nonzeroes:
    print("{} = {}".format(name, value))

# Actual script
cplex = ampls.CPLEXDrv() # Create the driver
cplexModel = cplex.loadModel(MODEL) # Load the model
doStuff(cplexModel) # Call the (generic) function

gurobi = ampls.GurobiDrv() # Create the driver
gurobiModel = gurobi.loadModel(MODEL) # Load the model
doStuff(gurobiModel) # Call the (generic) function
