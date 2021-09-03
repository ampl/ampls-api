#ifdef USE_cplex
//#include "cplex_interface.h"
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

const char* MODELNAME = "testmodel.nl";


double doStuff(ampls::AMPLModel& m, const char *name) 
{
  int n = m.getNumVars();
  std::vector<int> indices(n);
  std::vector<double> values(n);
  for (int i = 0; i < n; i++)
  {
    indices[i] = i;
    values[i] = i % 2 == 0 ? 1.0 : -1.0;
  }
  m.addConstraint(indices.size(), indices.data(), values.data(),
    ampls::CutDirection::LE, 1, "additional");
  // Start the optimization process
  m.optimize();
  std::cout << m.getRecordedEntities(false) << "\n";

  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", name, obj);

  ampls::Status::SolStatus s = m.getStatus();
  switch (s)
  {
    case ampls::Status::OPTIMAL:
      printf("Optimal.\n");
      break;
    case ampls::Status::INFEASIBLE:
      printf("Infeasible.\n");
      break;
    case ampls::Status::UNBOUNDED:
      printf("Unbounded.\n");
      break;
    default:
      printf("Status: %d\n", s);
  }
  // Get the solution vector
  std::vector<double> solution = m.getSolutionVector();
  int nnz = 0;
  for (int i = 0; i < solution.size(); i++)
    if (solution[i] != 0) nnz++;
  printf("\nNumber of non zeroes = %d\n", nnz);

  // Write the AMPL sol file
  m.writeSol();
  return obj;
}
int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);
  

  ampl::AMPL a;
  a.eval("option version;");
#ifdef USE_gurobi
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel g = gurobi.loadModel(buffer);
  printf("I Have %d constraints\n", g.getIntAttr(GRB_INT_ATTR_NUMCONSTRS));
  // Use it as generic model
  doStuff(g, "gurobi");
  double mipgap = g.getDoubleAttr(GRB_DBL_ATTR_MIPGAP);
  printf("\nFINAL MIP GAP=%f\n", mipgap);
  printf("I Have %d constraints\n", g.getIntAttr(GRB_INT_ATTR_NUMCONSTRS));
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
