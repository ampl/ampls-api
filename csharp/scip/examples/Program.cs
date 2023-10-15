using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using gsharp;

namespace gsharp_test
{
  class Program
  {
    private class CB : gsharp.GRBCallback
    {
      int count = 0;
      public override int run(int whereFrom)
      {
        var map = getVarMap();
       // foreach (var m in map)
       //   System.Diagnostics.Debug.WriteLine("{0} : {1}", m.Key, m.Value);
        string[] names = new string[2];
        for (int i = 1; i < 3; i++)
          names[i-1] = Utils.getAMPLVarName("x", i, 1);
        double[] coeffs = new double[] { 4.5, 5 };
        vector_string v = new vector_string(names);
        
        if (whereFrom == gsharp_c.GRB_CB_MESSAGE)
        {
          string msg = getMessage();
          Console.WriteLine(msg);
        }
        if ((whereFrom == gsharp_c.GRB_CB_MIPSOL_SOL)
          || (whereFrom == gsharp_c.GRB_CB_MIPNODE))
        {
          double[] sol = new double[10];
          getSolution(10, sol);
        }
        if (whereFrom == gsharp_c.GRB_CB_MIP)
        {
          addLazy(v, coeffs, '0', 5);
          count++;
          Console.WriteLine("GRB_CB_MIP #{0}!", count);
          
          if (count == 10)
            return 1;
        }
        return 0;
      }
    }

    static void Main(string[] args)
    {
      scipDrv g = new scipDrv();
      
      var m = g.loadModel(@"D:\Development\AMPL\escrow-ampls\solvers_dist\test\models\tsp.nl");
      int nvars = m.getNumVars();
      CB b = new CB();
      m.setCallback(b);
      double obj = m.optimize();
      double[] sol = new double[nvars];
      m.getSolution(0, nvars, sol);
      m.writeSol();
      double ff = m.getDoubleAttr(gsharp_c.GRB_DBL_ATTR_OBJVAL);
      var map = m.getVarMap();
      foreach (var item in map)
        Console.WriteLine("{0}: {1}", item.Key, item.Value);
    }
  }
}
