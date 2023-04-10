#include "copt_interface.h"
#include "ampls/ampls.h"

#include "copt.h" 

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "testmodel.nl";

/* Demonstrates how to load a model using ampls copt interface,
   and how to obtain basic information with ampls routines and
   via copt c library.
*/


int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // Load a model using copt driver
  ampls::CoptDrv copt;
  ampls::CoptModel model = copt.loadModel(buffer);

  // Start the optimization process
  model.optimize();
  // Get the pointer to the native model pointer, used with 
  // the copt C library functions below
  auto prob = model.getCOPTmodel();

   // Access objective value in various ways
  // 1 via ampls interface
  double obj = model.getObj();
  printf("Objective via ampls generic interface: %f\n", obj);
  // 2 Access copt attribute with shortcut function getDoubleAttr
  obj = model.getDoubleAttr(COPT_DBLATTR_LPOBJVAL);
  printf("Objective via shortcut: %f\n", obj);
  // 3 Use Copt C API directly
  COPT_GetDblAttr(prob, COPT_DBLATTR_LPOBJVAL, &obj);
  printf("Objective from copt: %f\n", obj);

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
  
  // Get solution vector via Copt C API
  int nc;
  COPT_GetIntAttr(prob, COPT_INTATTR_COLS, &nc);
  double* varsFromCopt = new double[nc];
  COPT_GetSolution(prob, varsFromCopt);

  // Get inverse map and display the variables with solver ordering
  auto gf = model.getVarMapInverse();
  printf("\nSolution vector ordered by solver\n");
  for (int i = 0; i < nc; i++)
  {
    if (vars[i] != 0)
      printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
  }
  delete[] varsFromCopt;
  return 0;
}
