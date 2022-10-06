#ifdef USE_cplex
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_x-gurobi
#include "x-gurobi_interface.h"
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

#ifdef USE_x-gurobi
    template<> ampls::GurobiDirectModel exportModel<ampls::GurobiDirectModel>(ampl::AMPL& a) {
      doExport(a);
      ampls::GurobiDirectDrv gurobi;
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
    std::cout << g.getRecordedEntities() << "\n";
    a.eval(g.getRecordedEntities());
  }
};

void printStatistics(ampl::AMPL& ampl) {
  printf("AMPL: I have %d variables and %d constraints\n",
    static_cast<int>(ampl.getValue("_nvars").dbl()),
    static_cast<int>(ampl.getValue("_ncons").dbl()));
  printf("My variables are\n");
  ampl.eval("display _VARS;");
}

void printStatistics(ampls::AMPLModel& m, const char* name)
{
  printf("\n%s: I have %d variables and %d constraints\n",
    name, m.getNumVars(), m.getNumCons());
  printf("%s: objective=%f\n", name, m.getObj());
}

template <class T> void doStuff(const char* name)
{
  ampl::AMPL ampl;
  
  ampl.eval("var x >=0 integer; var y>=0 integer;");
  ampl.eval("maximize z: x+y + to_come;");
  ampl.eval("constraint: x+2*y+ to_come<=4;");

  printStatistics(ampl);

  T model = AMPLAPIInterface::exportModel<T>(ampl);
  model.optimize();
  printStatistics(model, name);

  // Create a new constraint using the solver interface
  int n = model.getNumVars();
  std::vector<int> indices(n);
  std::vector<double> coeff(n, 1);
  for (int i = 0; i < n; i++)
    indices[i] = i;
  
  // Add it to the solver and records it for AMPL
  model.record(model.addConstraint(n, indices.data(), coeff.data(), ampls::CutDirection::LE, n));
  model.optimize();
  printStatistics(model, name);

  // Add a variable that does not appear in the constraints matrix
  // but with a coefficient of 100 in the objective
  model.record(model.addVariable(0, NULL, NULL, 0, 10, 100, ampls::VarType::Integer));
  model.optimize();
  printStatistics(model, name);

  AMPLAPIInterface::importModel(ampl, model);
  printStatistics(ampl);
}


int main(int argc, char** argv) {
#ifdef USE_cplex
  doStuff<ampls::CPLEXModel>("cplex");
#endif
#ifdef USE_gurobi
  doStuff<ampls::GurobiModel>("gurobi");
#endif
#ifdef USE_x-gurobi
  doStuff<ampls::GurobiDirectModel>("gurobidirect");
#endif
#ifdef  USE_xpress
  doStuff<ampls::XPRESSModel>("xpress");
#endif 
  return 0;
 
}
