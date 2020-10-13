#include "gurobi_interface.h"
#include "ampls/ampls.h"

#include "gurobi_c.h" 

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "tsp.nl";

/* Demonstrates how to load a model using ampls gurobi interface,
   and how to obtain basic information with ampls routines and
   via gurobi c library.
*/


int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel model = gurobi.loadModel(buffer);

  // Start the optimization process
  model.optimize();
  // Get the pointer to the native model pointer, used with 
  // the gurobi C library functions below
  GRBmodel* grb = model.getGRBmodel();

   // Access objective value in various ways
  // 1 via ampls interface
  double obj = model.getObj();
  printf("Objective via ampls generic interface: %f\n", obj);
  // 2 Access gurobi attribute with shortcut function getDoubleAttr
  obj = model.getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  printf("Objective via shortcut: %f\n", obj);
  // 3 Use Gurobi C API directly
  GRBgetdblattr(grb, GRB_DBL_ATTR_OBJ, &obj);
  printf("Objective from gurobi: %f\n", obj);

  // Get solution vector via generic interface
  std::vector<double> vars = model.getSolutionVector();
  // Get map and display the variables ordered as AMPL stores them
  auto fg = model.getVarMap();
  double value;
  printf("\nSolution vector ordered by AMPL definition\n");
  for (auto r : fg)
  {
    value = vars[r.second];
    if (value != 0)
      printf("Index: %i AMPL: %s=%f\n", r.second, r.first.data(), value);
  }
  
  // Get solution vector via Gurobi C API
  int nc;
  GRBgetintattr(grb, GRB_INT_ATTR_NUMVARS, &nc);
  double* varsFromGurobi = new double[nc];
  GRBgetdblattrarray(grb, GRB_DBL_ATTR_X, 0, nc, varsFromGurobi);

  // Get inverse map and display the variables with solver ordering
  auto gf = model.getVarMapInverse();
  printf("\nSolution vector ordered by solver\n");
  for (int i = 0; i < nc; i++)
  {
    if (vars[i] != 0)
      printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
  }
  delete[] varsFromGurobi;
  return 0;
}
