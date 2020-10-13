#ifdef USE_cplex 
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_xpress
#include "xpress_interface.h"
#endif

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "tsp.nl";

double doStuff(ampls::AMPLModel& m, const char *name) 
{
  // Start the optimization process
  m.optimize();
  printf("\n");
  // Get the generic status
  ampls::Status::SolStatus s = m.getStatus();
  switch (s)
  {
  case ampls::Status::OPTIMAL:
    printf("Optimal ");
    break;
  case ampls::Status::INFEASIBLE:
    printf("Infeasible ");
    break;
  case ampls::Status::UNBOUNDED:
    printf("Unbounded ");
    break;
  default:
    printf("Status (%d) ", s);
  }
  // Get the objective value
  double obj = m.getObj();
  printf("solution with %s=%f\n", name, obj);
  

  // Get the solution vector and count the non-zeros
  std::vector<double> solution = m.getSolutionVector();
  int nnz = 0;
  for (int i = 0; i < solution.size(); i++)
    if (solution[i] != 0) nnz++;
  printf("Number of non zeroes = %d\n", nnz);
  // Write the AMPL sol file
  char BUFFER[1024];
  sprintf(BUFFER, "%s-%s.sol", m.getFileName().c_str(), name);
  printf("Writing solution file to: %s\n", BUFFER);
  m.writeSol(BUFFER);
  return obj;
}

int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

#ifdef USE_gurobi
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel g = gurobi.loadModel(buffer);
  // Use it as generic model
  doStuff(g, "gurobi");
#endif

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
  return 0;
}
