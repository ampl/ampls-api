#include <iostream>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"

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

template <class T> void doStuff()
{
  ampl::AMPL ampl;
  
  ampl.eval("var x >=0 integer; var y>=0 integer;");
  ampl.eval("maximize z: x+y + to_come;");
  ampl.eval("constraint: x+2*y+ to_come<=4;");

  printStatistics(ampl);

  T model = ampls::AMPLAPIInterface::exportModel<T>(ampl);
  model.optimize();
  printStatistics(model, model.driver());

  // Create a new constraint using the solver interface
  int n = model.getNumVars();
  std::vector<int> indices(n);
  std::vector<double> coeff(n, 1);
  for (int i = 0; i < n; i++)
    indices[i] = i;
  
  // Add it to the solver and records it for AMPL
  model.record(model.addConstraint(n, indices.data(), coeff.data(), ampls::CutDirection::LE, n));
  model.optimize();
  printStatistics(model, model.driver());

  // Add a variable that does not appear in the constraints matrix
  // but with a coefficient of 100 in the objective
  model.record(model.addVariable(0, NULL, NULL, 0, 10, 100, ampls::VarType::Integer));
  model.optimize();
  printStatistics(model, model.driver());

  ampls::AMPLAPIInterface::importModel(ampl, model);
  printStatistics(ampl);
}


int main(int argc, char** argv) {
#ifdef USE_copt
  doStuff<ampls::CoptModel>();
#endif 
#ifdef USE_cplexmp
  doStuff<ampls::CPLEXModel>();
#endif
#ifdef USE_gurobi
  doStuff<ampls::GurobiModel>();
#endif
#ifdef USE_xpress
  doStuff<ampls::XPRESSModel>();
#endif 
#ifdef USE_cbcmp
  doStuff<ampls::CbcModel>();
#endif 
  return 0;
 
}
