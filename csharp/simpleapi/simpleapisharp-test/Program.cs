using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using simpleapisharp;
using Where = simpleapisharp.Where.CBWhere;
using Value = simpleapisharp.Value.CBValue;

namespace simpleapiharp_test
{
    class Program
    {
        private class GCB : GenericCallback
        {
            public override int run()
            {
                var f = getAMPLWhere();
                switch (f)
                {
                    case Where.msg:
                        Console.WriteLine(getMessage());
                        break;
                    case Where.presolve:
                        Console.WriteLine("Presolve: {0} coefficient changed, {1} columns and {2} rows eliminated",
                            getValue(Value.pre_coeffchanged).integer, getValue(Value.pre_delcols).integer,
                            getValue(Value.pre_delrows).integer);
                        break;
                    case Where.mipnode:
                    case Where.mipsol:
                        try
                        {
                            Console.WriteLine("MIP Objective = {0}", getObj());
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e.Message);
                        }
                        break;
                    case Where.notmapped:
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
            const string model = @"D:\Development\AMPL\solvers-public\test\models\tsp.nl";
            CPLEXDrv cpx = new CPLEXDrv();
            var mc = cpx.loadModel(model);
            DoStuff(mc);
            GurobiDrv grb = new GurobiDrv();
            var mg = grb.loadModel(model);
            DoStuff(mg);

           
        }
    }
}
