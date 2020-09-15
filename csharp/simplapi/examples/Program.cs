using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace simpleapi_test
{
  class Program
  {
    const string MODEL = @"D:\Development\AMPL\escrow-simpleapi\solvers_dist\test\models\tsp.nl";
    static void Main(string[] args)
    {
      gsharp.GurobiDrv grb = new gsharp.GurobiDrv();
      var gurobi = grb.loadModel(MODEL);
      cpxsharp.CPLEXDrv cpx = new cpxsharp.CPLEXDrv();
      var cplex = cpx.loadModel(MODEL);
      cpxsharp.AMPLModel m = cplex;
      gsharp.AMPLModel m2;

    }
  }
}
