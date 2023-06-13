#include "cbcmp_interface.h"
#include "ampls/ampls.h"


#define CBC_EXTERN_C
#include "CbcModel.hpp"
#include "CbcSolver.hpp"
#include "Cbc_C_Interface.h"

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "testmodel.nl";

/* Demonstrates how to load a model using ampls cbc interface,
   and how to obtain basic information with ampls routines and
   via cbc c library.
*/


int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // Load a model using cbc driver
  ampls::CbcDrv cbcdrv;
  ampls::CbcModel model = cbcdrv.loadModel(buffer);

  // Start the optimization process
  model.optimize();
  // Get the pointer to the native model pointer, used with 
  // the cbc C library functions below
  Cbc_Model* cbc = model.getCBCmodel();

   // Access objective value in various ways
  // 1 via ampls interface
  double obj = model.getObj();
  printf("Objective via ampls generic interface: %f\n", obj);
  // 2 Access 
 // obj = model.getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  printf("Objective via shortcut: %f\n", obj);
  // 3 Use Cbc C API directly
  obj = Cbc_getObjValue(cbc);
  printf("Objective from cbc: %f\n", obj);

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
  
  // Get solution vector via Cbc C API
  int nc = model.getNumVars();
  const double* varsFromCbc = Cbc_getColSolution(cbc);

  // Get inverse map and display the variables with solver ordering
  auto gf = model.getVarMapInverse();
  printf("\nSolution vector ordered by solver\n");
  for (int i = 0; i < nc; i++)
  {
    if (vars[i] != 0)
      printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
  }
  return 0;
}
