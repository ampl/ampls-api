#include <string>
#include <exception>
#include <cassert>
#include <iostream> // for ostringstream

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"

void createModel(ampl::AMPL& ampl) {
  ampl.eval("var x binary; var y binary; var z binary;"
    "minimize TotalSum: z + 1;"
    "subj to C1 : x + y >= 1;");
}

template <class T> T solveModel(ampl::AMPL& ampl) {
    T model = ampls::AMPLAPIInterface::exportModel<T>(ampl);
    model.setOption("outlev", 1);
    model.setOption("sol:stub", "stub");
    try {
      model.setOption("sol:poolgap", 0.1);
    }
    catch (const ampls::AMPLSolverException& e) {
      printf(e.what());
    }
    // Have to refresh to make the driver aware of the options
    model.refresh();
    // In most solvers, the driver "solve" function
    // is used; this makes the calling program call the drivers' solve
    // function, that in many cases will contain code specifically for
    // handling multiple solutions
    model.optimize();
    return model;
}

template <class T> void example() {
  ampl::AMPL ampl;
  createModel(ampl);
  auto m = solveModel<T>(ampl);
  ampls::AMPLAPIInterface::importModel(ampl, m);

  int nsol = static_cast<int>(ampl.getValue("TotalSum.nsol").dbl());
  printf("Gotten %d solutions\n", nsol);

  // Load solutions into the AMPL object and display them
  char BUFFER[24];
  for (int i = 1; i <= nsol; i++) {
    sprintf(BUFFER, "solution stub%i.sol;", i);
    ampl.eval(BUFFER);
    std::cout << ampl.getData("x,y,z").toString();
  }
}

int main(int argc, char** argv) {
  #ifdef USE_gurobi
  example<ampls::GurobiModel>();
  #endif

  #ifdef USE_xpress
  // Note XPRESS is not supported (yet) due to its very specific
  // handling of multiple solutions
  example<ampls::XPRESSModel>();
  #endif

  #ifdef USE_cbcmp
  example<ampls::CbcModel>();
  #endif

  #ifdef USE_copt
  example<ampls::CoptModel>();
  #endif

  #ifdef USE_cplex
  example<ampls::CPLEXModel>();
  #endif
#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif
    return 0;
}
