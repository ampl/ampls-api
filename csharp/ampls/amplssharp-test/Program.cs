using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ampls;
using Where = ampls.Where.CBWhere;
using Value = ampls.Value.CBValue;

namespace amplsharp_test
{
    class Program
    {
        private class GCB : GenericCallback
        {
            public override int run()
            {
                var f = getAMPLSWhere();
                switch (f)
                {
                    case Where.MSG:
                        Console.WriteLine(getMessage());
                        break;
                    case Where.PRESOLVE:
                        Console.WriteLine("Presolve: {0} coefficient changed, {1} columns and {2} rows eliminated",
                            getValue(Value.PRE_COEFFCHANGED).integer, getValue(Value.PRE_DELCOLS).integer,
                            getValue(Value.PRE_DELROWS).integer);
                        break;
                    case Where.MIPNODE:
                    case Where.MIPSOL:
                        try
                        {
                            Console.WriteLine("MIP Objective = {0}", getObj());
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e.Message);
                        }
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
        static void DoStuff(AMPLModel m)
        {
            // Get the number of variables
            int nvars = m.getNumVars();
            GCB gcb = new GCB();
            m.setCallback(gcb);
            double obj = m.optimize();
            var sol = m.getSolutionVector().Where(a => a != 0).ToList();
            Console.WriteLine($"Status: {m.getStatus().ToString()}");
            Console.WriteLine($"Solution of {m.GetType().Name}={m.getObj()}, nnz={sol.Count()}");
            var map = m.getVarMapInverse();
            Console.WriteLine($"First 10 non zeroes:");
            for (int i = 0; i < Math.Min(sol.Count, 10); i++)
                Console.WriteLine($"{map[i]}: {sol[i]}");
        }

        static void Main(string[] args)
        {
            const string model = @"D:\Development\AMPL\ampls-api\test\models\model.nl";
           // CPLEXDrv cpx = new CPLEXDrv();
            //var mc = cpx.loadModel(model);
           // DoStuff(mc);
            GurobiDrv grb = new GurobiDrv();
            var mg = grb.loadModel(model);
            DoStuff(mg);

           
        }
    }
}
