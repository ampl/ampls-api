using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using simpleapisharp;


namespace simpleapiharp_test
{
    class Program
    {
        private class GCB : GenericCallback
        {
            public override int run()
            {
                var f = getAMPLType();
                switch (f)
                {
                    case Where.msg:
                        //Console.WriteLine(getMessage());
                        break;
                    case Where.presolve:
                        //Console.WriteLine("Presolve!");
                        break;
                    case Where.mipnode:
                    case Where.mipsol:
                        try
                        {
                            Console.WriteLine("MIP Objective = {0}", getObjective());
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e.Message);
                        }
                        break;
                    case Where.notmapped:
                        Console.WriteLine($"Not mapped! Where = {getWhere()}");
                        break;

                    default:
                        return 0;
                }
                return 0;
            }
        }
        static void DoStuff(AMPLModel m)
        {
            int nvars = m.getNumVars();
            GCB gcb = new GCB();
            m.setCallback(gcb);
            double obj = m.optimize();
            var sol = m.getSolutionVector().Where(a => a != 0).ToList();
            Console.WriteLine($"Solution of {m.GetType().Name}={m.getObj()}, nnz={sol.Count()}");
            var map = m.getVarMapInverse();
            for (int i = 0; i < sol.Count; i++)
                Console.WriteLine($"{map[i]}: {sol[i]}");
            m.writeSol();
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
