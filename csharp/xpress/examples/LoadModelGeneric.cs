using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ampls;

namespace ampls_examples
{
  class LoadModelGeneric
  {
    public void run() { 

    GurobiDrv g = new GurobiDrv();
    var m = g.loadModel(@"D:\Development\AMPL\escrow-ampls\solvers_dist\test\models\tsp.nl");
    DoStuff(m, "gurobi");
    }

    private void DoStuff(AMPLModel m, string name)
    {
      m.setAMPLParameter(SolverParams.SolverParameters.DBL_MIPGap, 0.2);
      m.optimize();

      // Print model solution result
      string s = Enum.GetName(typeof(Status.SolStatus), m.getStatus());
      double obj = m.getObj();
      Console.WriteLine($"\n{s} solution with {name}={obj}");

      // Get the solution vector and count the non-zeros
      int nvars = m.getNumVars();
      double[] solution = new double[nvars];
      m.getSolution(0, nvars, solution);

      int nnz = solution.Where(x => x != 0).Count();
      Console.WriteLine($"Number of non zeros = {nvars}");

      string solFileName = $"{m.getFileName()}-{name}.sol";
      Console.WriteLine($"Writing solution file to {solFileName}");
      m.writeSol(solFileName);
    }

      
    
    
    }
  }
}
