using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ampls;
using Where = ampls.Where.CBWhere;

namespace gurobisharp_test
{
  class Program
  {
    private class CB : ampls.GenericCallback
    {
      
      int count = 0;
      public override int run()
      {
        var map = getVarMap();
       // foreach (var m in map)
       //   System.Diagnostics.Debug.WriteLine("{0} : {1}", m.Key, m.Value);
        string[] names = new string[2];
        for (int i = 1; i < 3; i++)
          names[i-1] = Utils.getAMPLVarName("x", i, 1);
        double[] coeffs = new double[] { 4.5, 5 };
        vector_string v = new vector_string(names);
        /*
        if (where == gsharp_c.GRB_CB_MESSAGE)
        {
          string msg = getMessage();
          Console.WriteLine(msg);
        }
        if ((where == gsharp_c.GRB_CB_MIPSOL_SOL)
          || (where == gsharp_c.GRB_CB_MIPNODE))
        {
          double[] sol = new double[10];
          getSolution(10, sol);
        }
        if (where == gsharp_c.GRB_CB_MIP)
        {
          addLazy(v, coeffs, '0', 5);
          count++;
          Console.WriteLine("GRB_CB_MIP #{0}!", count);
          
          if (count == 10)
            return 1;
        }*/
        return 0;
      }
    }

    private class GCB : GurobiCallback
    {

      public override int run()
      {
        var f = getAMPLWhere();
        System.Console.WriteLine($"Elapsed time {getValue(Value.CBValue.RUNTIME).dbl}");
        switch(f)
        {
          case Where.LPSOLVE:
            Console.WriteLine(getMessage());
            break;
          case Where.PRESOLVE:
          //Console.WriteLine("Presolve!");
            break;
          case Where.MIPNODE:
          case Where.MIPSOL:
            Console.WriteLine("MIP Objective = {0}", getObj());
            break;
          case Where.NOTMAPPED:
            Console.WriteLine($"Not mapped! Where = {getWhereString()}");
            break;
          default:
            return 0;
        }
        return 0;
      }
    }

    static void Main(string[] args)
    {
      GurobiDrv g = new GurobiDrv();
      
            try
            {
                var m = g.loadModel(@"D:\Development\AMPL\ampls-api\test\models\tsp.nl");

                int nvars = m.getNumVars();
                //CB cb = new CB();
                //m.setCallback(cb);

                GCB gcb = new GCB();
                m.setCallback(gcb);
                double obj = m.optimize();
                Console.WriteLine("Solution with CPLEX={0}", m.getObj());
                double[] sol = new double[nvars];
                m.getSolution(0, nvars, sol);
                m.writeSol();
               // var map = m.getVarMap();
               // foreach (var item in map)
               // {
                //    if (sol[item.Value] != 0)
                //        Console.WriteLine("{0}: {1}", item.Key, sol[item.Value]);
               // }
            }
            catch (Exception  ex)
            {
                Console.WriteLine("exception caught!\r\n" + ex.Message);
            }
    }
  }
}
