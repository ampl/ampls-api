#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "tsp.nl";


double doStuff(ampls::AMPLModel& m) 
{
  // Set parameter with common mapping
  m.setAMPLsParameter(ampls::SolverParams::DBL_MIPGap, 0.1);

  // Start the optimization process
  m.optimize();
  printf("\n%s solution ",m.driver());
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
  printf("%s: Relative MIP gap=%f\n", m.driver(), m.getAMPLsDoubleAttribute(ampls::SolverAttributes::DBL_RelMIPGap));

  // Get the solution vector and count the non-zeros
  std::vector<double> solution = m.getSolutionVector();
  int nnz = 0;
  for (int i = 0; i < solution.size(); i++)
    if (solution[i] != 0) nnz++;
  printf("%s: Number of non zeroes=%d\n", m.driver(), nnz);
  // Write the AMPL sol file
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

  std::vector<std::string>  options = { "return_mipgap=3"};
#ifdef USE_xgurobi
  // Load a model using gurobi driver
  ampls::XGurobiDrv gurobi;
  gurobi.setOptions(options);
  ampls::XGurobiModel g = gurobi.loadModel(buffer);
  // Use it as generic model
  doStuff(g);
#endif
#ifdef USE_xpressmp
  // Load a model using CPLEX driver
  ampls::XPRESSDrv xpress;
  //xpress.setOptions(options);
  ampls::XPRESSModel x = xpress.loadModel(buffer);
  // Use it as generic model
  doStuff(x);
#endif

#ifdef USE_cplexmp
  // Load a model using CPLEX driver
  ampls::CPLEXDrv cplex;
  cplex.setOptions(options);
  ampls::CPLEXModel c = cplex.loadModel(buffer);
  // Use it as generic model
  doStuff(c);
#endif
  



  return 0;
}
