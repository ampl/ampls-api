import sys
import amplpy_ampls as ampls
from tsp_helpers import tsp_model

statuses = {
    ampls.Status.OPTIMAL: "optimal",
    ampls.Status.INFEASIBLE: "infeasible",
    ampls.Status.INTERRUPTED: "interrupted",
    ampls.Status.LIMIT_ITERATION: "limit iteration",
    ampls.Status.LIMIT_NODE: "limitnode",
    ampls.Status.LIMIT_SOLUTION: "limitsolution",
    ampls.Status.LIMIT_TIME: "limittime",
    ampls.Status.UNBOUNDED: "unbounded",
    ampls.Status.UNKNOWN: "unknown",
    ampls.Status.NOTMAPPED: "not mapped"
}

# Generic function doing the optimization and reading the results


def doStuff(a):
    # Optimize with default settings
    a.optimize()
    # Print the status (using the map above to have a description)
    print("Model status", statuses[a.getStatus()])
    # Print the objective function
    print("Objective:", a.getObj())
    # Get the solution vector
    d = a.getSolutionVector()
    # Print the non-zeroes, by getting a map
    # Note that this will only work if the col file has been generated
    map = a.getVarMapInverse()
    nonzeroes = [(map[index], d[index]) for index in map if d[index] != 0]
    for (name, value) in nonzeroes:
        print("{} = {}".format(name, value))


ampl = tsp_model('tsp_51_1.txt')

cm = ampl.exportCplexModel()
doStuff(cm)

gm = ampl.exportGurobiModel()
doStuff(gm)
