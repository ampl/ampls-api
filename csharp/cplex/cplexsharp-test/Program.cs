using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ampls;
using Where = ampls.Where.CBWhere;

namespace cpxsharp_test
{
  class Program
  {
    private class MyCPLEXCallback : CPLEXCallback
    {
      public override int run()
      {

        var w = getWhere();
        Console.WriteLine($"Where = {w}, which is {getWhereString()}");
        if (w == cpxsharp_c.CPX_CALLBACK_PRESOLVE)
            Console.WriteLine("MIP!");
        return 0;
      }
    }

    private class GCB : GenericCallback
    {

      public override int run()
      {
        var f = getAMPLWhere();
        Console.WriteLine((int)f);
        switch (f)
        {
          case Where.MSG:
           // Console.WriteLine(getMessage());
            break;
          case Where.PRESOLVE:
            //Console.WriteLine("Presolve!");
            break;
          case Where.MIPNODE:
          case Where.MIPSOL:
          //  Console.WriteLine("MIP Objective = {0}", getObjective());
            break;
          case Where.NOTMAPPED:
            //Console.WriteLine($"Not mapped! Where = {getWhere()}");
            break;

          default:
            return 0;
        }
        return 0;
      }
    }
        
      static void Main(string[] args)
    {
      string modelFile = ((args != null) && (args.Length > 0)) ? args[0]
           : "../../../../../ampls-api/test/models/tsp.nl";
            CPLEXDrv g = new CPLEXDrv();
            try
            {
                var m = g.loadModel(modelFile);

                int nvars = m.getNumVars();
                // CB cb = new CB();
                //m.setCallback(cb);

                MyCPLEXCallback cplexcallback = new MyCPLEXCallback();
                m.setCallback(cplexcallback);
                //double obj = m.optimize();
                var env = m.getCPXENV();
                var cpx = m.getCPXLP();
                cpxsharp_c.CPXmipopt(env, cpx);
                
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
