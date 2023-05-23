import amplpy_gurobi as ampls
from amplpy import AMPL



def define_model():
    ampl = AMPL()
    ampl.eval('var x binary; var y binary; var z binary;'
              'minimize TotalSum: z + 1;'
              'subj to C1: x+y >= 1;')
    return ampl

def solve(ampl: AMPL):
    mod = ampl.export_model('gurobi')


    # Use AMPL driver parameter
    #mod.set_option("lim:solution", 5)
    mod.set_option("outlev", 1)
    mod.set_option("sol:poolgap", 0.1)
    mod.set_option("sol:stub", "test_multi")
    # use ampls mapping
    mod.set_ampls_parameter(ampls.SolverParams.INT_SolutionLimit, 5)

    # Use gurobi parameter
    mod.set_param(ampls.GRB_INT_PAR_SOLUTIONLIMIT, 5)

    mod.optimize()

   
    print(mod.get_obj())


if __name__ == "__main__":
    a = define_model()
    solve(a)