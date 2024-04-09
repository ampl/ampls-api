#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cassert>
#include <string>  



// This example shows how to:
// 1. load a model from an NL file, passing an option at loading time
// 2. solve it 
// 3. get basic information about the solution
// 4. write out a solution file ready to be imported in AMPL
// Note that this function could have included the ampls::AMPLModel creation
// but having the base class as a parameter helps intellisense
void doStuff(ampls::AMPLModel& m) {
  // Set parameter with common mapping
  m.setAMPLParameter(ampls::SolverParams::DBL_MIPGap, 0.1);
  // Start the optimization process
  m.optimize();
  printf("\n%s solution ", m.driver());
  // Get the generic status
  ampls::Status::SolStatus s = m.getStatus();
  assert(s == ampls::Status::OPTIMAL);
  switch (s)
  {
  case ampls::Status::OPTIMAL:
    printf("optimal\n");
    break;
  case ampls::Status::INFEASIBLE:
    printf("infeasible\n");
    break;
  case ampls::Status::UNBOUNDED:
    printf("unbounded\n");
    break;
  default:
    printf("Status (%d)\n", s);
  }
  // Get the objective value
  double obj = m.getObj();
  printf("%s: Objective=%f\n", m.driver(), obj);
  // Get the MIP gap using its generic alias
  printf("%s: Relative MIP gap=%f\n", m.driver(), m.getAMPLDoubleAttribute(ampls::SolverAttributes::DBL_RelMIPGap));

  // Get the solution vector and count the non-zeros
  auto solution = m.getSolutionVector();
  int nnz = 0;
  for (int i = 0; i < solution.size(); i++)
    if (solution[i] != 0) nnz++;
  printf("%s: Number of non zeroes=%d\n", m.driver(), nnz);
  assert(nnz==51);

  // Set an option of the solver driver (in this case, instructing it to
  // return the MIP gap when communicating the solution back to AMPL)
  m.setOption("return_mipgap", 3);

  // Write a solution file ready to be imported in AMPL, that includes the MIP gap
  char BUFFER[100];
  sprintf(BUFFER, "%s-%s.sol", m.getFileName().c_str(), m.driver());
  printf("%s: Writing solution file to: %s\n\n\n", m.driver(), BUFFER);
  m.writeSol(BUFFER);
}

template <class T> void example() {
  assert(1);
  const char* MODELNAME = "tsp.nl";
  std::string md(MODELS_DIR);
  md += MODELNAME;
  const char* options[2] = { "outlev=1", NULL };
  
  T model = ampls::AMPLModel::load<T>(md.c_str(), options);
  doStuff(model);
}

int main(int argc, char** argv) {
#ifdef USE_gurobi
  example<ampls::GurobiModel>();
#endif

#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif

#ifdef USE_cplex
  example< ampls::CPLEXModel>();
#endif

#ifdef USE_copt
  example<ampls::CoptModel>();
#endif

#ifdef USE_cbcmp
  example<ampls::CbcModel>();
#endif

#ifdef USE_highs
  example<ampls::HighsModel>();
#endif

#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif
  return 0;
}
