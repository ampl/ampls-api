#ifdef USE_cplex
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_xpress
#include "xpress_interface.h"
#endif

#include <iostream>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"

namespace AMPLAPIInterface
{
  namespace impl {

    void doExport(ampl::AMPL& a) {
      a.eval("option auxfiles cr;");
      a.eval("write g___modelexport___;");
    }

    template <class T> T exportModel(ampl::AMPL& a);
#ifdef USE_gurobi
    template<> ampls::GurobiModel exportModel<ampls::GurobiModel>(ampl::AMPL& a) {
      doExport(a);
      ampls::GurobiDrv gurobi;
      return gurobi.loadModel("___modelexport___.nl");
    }
#endif

#ifdef USE_cplex
    template<> ampls::CPLEXModel exportModel<ampls::CPLEXModel>(ampl::AMPL& a) {
      doExport(a);
      ampls::CPLEXDrv cplex;
      return cplex.loadModel("___modelexport___.nl");
    }
#endif

#ifdef USE_xpress
    template<> ampls::XPRESSModel exportModel<ampls::XPRESSModel>(ampl::AMPL& a) {
      doExport(a);
      ampls::XPRESSDrv xpress;
      return xpress.loadModel("___modelexport___.nl");
    }
#endif
  }

  template <class T> T exportModel(ampl::AMPL& a) {
    return impl::exportModel<T>(a);
  }

  void importModel(ampl::AMPL& a, ampls::AMPLModel& g) {
    g.writeSol();
    a.eval("solution ___modelexport___.sol;");
    a.eval(g.getRecordedEntities());
  }

};

void declareModel(ampl::AMPL &a) {
  a.eval("option version;");
  a.eval("param nPatterns integer > 0;"
    "set PATTERNS = 1..nPatterns;  "
    "set WIDTHS;                   # finished widths"
    "param order{ WIDTHS } >= 0;    # rolls of width j ordered"
    "param overrun;                # permitted overrun on any width"
    "param rawWidth;               # width of raw rolls to be cut"
    "param rolls{ WIDTHS,PATTERNS } >= 0, default 0;"
   
    "var Cut{ PATTERNS } integer >= 0;  # raw rolls to cut in each pattern"
    "minimize TotalRawRolls : sum{ p in PATTERNS } Cut[p];"
    "subject to OrderLimits{ w in WIDTHS }:"
    "order[w] <= sum{ p in PATTERNS } rolls[w, p] * Cut[p] <= order[w] + overrun; ");
  a.eval("show Cut;");
  int overrun = 0;
  int nPatterns = 5;
  double roll_width = 110;
  double ordersWidths[] = { 20, 45, 50, 55, 75 };
  double ordersAmount[] = { 48, 35, 24, 10, 8 };
/*  int nPatterns = 49;
  double ordersWidths[] = { 1630, 1625, 1620, 1617, 1540, 1529, 1528, 1505, 1504, 1484, 1466,
    1450, 1283, 1017, 970, 930, 916, 898, 894, 881, 855, 844, 805, 787, 786, 780, 754,
    746, 707, 698, 651, 644, 638, 605, 477, 473, 471, 468, 460, 458, 453, 447, 441,
    422, 421, 419, 396, 309, 266 };
  double ordersAmount[] = { 172, 714, 110, 262, 32, 100, 76, 110,20, 58, 15, 10, 40, 50, 70, 8, 210, 395,
      49, 17, 20, 10, 718, 17, 710, 150, 34, 15, 122, 7, 10, 15, 10, 10, 4, 34, 25, 10, 908,
      161, 765, 21, 20, 318, 22, 382, 22, 123, 35 };
      */
  a.getParameter("nPatterns").set(nPatterns); 
  a.getParameter("overrun").set(overrun);
  a.getParameter("rawWidth").set(roll_width);
  ampl::DataFrame df(1, { "WIDTHS", "order" });
  df.setColumn("WIDTHS", ordersWidths, nPatterns);
  df.setColumn("order", ordersAmount, nPatterns);

}

template <class T> void doStuff()
{
  ampl::AMPL a;
  declareModel(a);
  auto m= AMPLAPIInterface::exportModel<T>(a);


}
int main(int argc, char** argv) {
#ifdef USE_gurobi
  doStuff<ampls::GurobiModel>();
#endif
  /*
#ifdef USE_cplex
  // Load a model using CPLEX driver
  ampls::CPLEXDrv cplex;
  ampls::CPLEXModel c = cplex.loadModel(buffer);
  // Use it as generic model
  doStuff(c, "cplex");
#endif

#ifdef USE_xpress
  // Load a model using CPLEX driver
  ampls::XPRESSDrv xpress;
  ampls::XPRESSModel x = xpress.loadModel(buffer);
  // Use it as generic model
  doStuff(x, "xpress");
#endif
*/
  return 0;
 
}
