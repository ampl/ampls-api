#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 
const char* MODELNAME = "tsp.nl";

// This example shows how to:
// 1. load a model from an NL file,
// 2. solve it 
// 3. get basic information about the solution
// 4. write out a solution file ready to be imported in AMPL


double doStuff(ampls::AMPLModel& m) 
{
  // Set parameter with common mapping
  m.setAMPLSParameter(ampls::SolverParams::DBL_MIPGap, 0.1);
  // Start the optimization process
  m.optimize();
  printf("\n%s solution ", m.driver());
  // Get the generic status
  ampls::Status::SolStatus s = m.getStatus();
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
  printf("%s: Relative MIP gap=%f\n", m.driver(), m.getAMPLSDoubleAttribute(ampls::SolverAttributes::DBL_RelMIPGap));

  // Get the solution vector and count the non-zeros
  auto solution = m.getSolutionVector();
  int nnz = 0;
  for (int i = 0; i < solution.size(); i++)
    if (solution[i] != 0) nnz++;
  printf("%s: Number of non zeroes=%d\n", m.driver(), nnz);
  
  // Set an option specific to the solver driver (in this case, instructing it to
  // return the MIP gap when communicating the solution back to AMPL)
  m.setOption("return_mipgap", 3);

  // Write a solution file ready to be imported in AMPL
  char BUFFER[1024];
  sprintf(BUFFER, "%s-%s.sol", m.getFileName().c_str(), m.driver());
  printf("%s: Writing solution file to: %s\n\n\n", m.driver(), BUFFER);
  m.writeSol(BUFFER);
  return obj;
}

int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

#ifdef USE_gurobi
  try {
    // Load a model using gurobi driver
    ampls::GurobiModel gurobimodel = ampls::AMPLModel::load<ampls::GurobiModel>(buffer);
    // Use it as generic model
    doStuff(gurobimodel);
  }
  catch (const ampls::AMPLSolverException& e) {
    printf(e.what());
  }
#endif

#ifdef USE_xpress
  try {
    // Load a model using XPRESS driver
    ampls::XPRESSModel xpressmodel = ampls::AMPLModel::load<ampls::XPRESSModel>(buffer);
    // Use it as generic model
    doStuff(xpressmodel);
  }
  catch (const ampls::AMPLSolverException& e) {
    printf(e.what());
  }
#endif

#ifdef USE_cplex
  try {
    // Load a model using CPLEX driver
    ampls::CPLEXModel cplexmodel = ampls::AMPLModel::load<ampls::CPLEXModel>(buffer);
    // Use it as generic model
    doStuff(cplexmodel);
  }
  catch (const ampls::AMPLSolverException& e) {
    printf(e.what());
  }
#endif
  
#ifdef USE_copt
  try{
    // Load a model using Copt driver
    ampls::CoptModel coptmodel = ampls::AMPLModel::load<ampls::CoptModel>(buffer);
    // Use it as generic model
    doStuff(coptmodel);
  }
  catch (const ampls::AMPLSolverException& e) {
    printf(e.what());
  }
#endif

#ifdef USE_cbcmp
  // Load a model using CBC driver
  ampls::CbcModel cbcmodel = ampls::AMPLModel::load<ampls::CbcModel>(buffer);
  // Use it as generic model
  doStuff(cbcmodel);
#endif

  return 0;
}
