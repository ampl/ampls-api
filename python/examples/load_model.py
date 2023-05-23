from tsp_helpers import tsp_model

import amplpy_gurobi 
#import amplpy_cplex
ampls = amplpy_gurobi



def doStuff(a):
    '''
    Generic function doing the optimization and reading the results
    '''
    # Optimize with default settings
    a.optimize()
    print("Model status:", a.get_status())
    # Print the objective function
    print("Objective:", a.get_obj())
    # Get the solution vector
    d = a.get_solution_vector()
    # Print the non-zeroes, by getting a map
    # Note that this will only work if the col file has been generated
    map = a.get_var_map_inverse()
    nonzeroes = [(map[index], d[index]) for index in map if d[index] != 0]
    for (name, value) in nonzeroes:
        print("{} = {}".format(name, value))
    return cm.get_solution_dict()


ampl = tsp_model('tsp_51_1.txt')

#cm = ampl.export_cplex_model()
#print(doStuff(cm))

gm = ampl.export_gurobi_model()
print(doStuff(gm))
