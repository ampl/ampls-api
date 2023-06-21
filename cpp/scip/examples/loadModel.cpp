#include "scip_interface.h"
#include "ampls/ampls.h"

#include "scip/scip.h" 

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "tspg96.nl";

/* Demonstrates how to load a model using ampls SCIP interface,
   and how to obtain basic information with ampls routines and
   via SCIP library.
*/
int main(int argc, char** argv) {
  std::string s = MODELS_DIR;
  s += MODELNAME;
  // Load a model using SCIP
  ampls::SCIPModel model = ampls::AMPLModel::load<ampls::SCIPModel>(s.c_str());

  // Start the optimization process
  model.optimize();
  // Get the pointer to the native model pointer, used with 
  // the SCIP C library functions below
  SCIP* scip = model.getSCIPmodel();

  // Access objective value in various ways
  // 1 via ampls interface
  double obj = model.getObj();
  printf("Objective via ampls generic interface: %f\n", obj);
  // 2 Use SCIP C API directly
  obj = SCIPgetPrimalbound(scip);
  printf("Objective from scip: %f\n", obj);

  // Get solution vector via generic interface
  std::vector<double> vars = model.getSolutionVector();
  // Get map and display the variables ordered as AMPL stores them
  auto fg = model.getVarMap();
  double value;
  int numvar = model.getNumVars();
  for (auto r : fg)
  {
    value = vars[r.second];
    if (value != 0)
      printf("Index: %i AMPL: %s=%f\n", r.second, r.first.data(), value);
  }
  
  // Get solution vector via SCIP C API
  int nc;
  nc = SCIPgetProbData(scip)->nvars;
  double* varsFromSCIP = new double[nc];

  // Get inverse map and display the variables with solver ordering
  auto gf = model.getVarMapInverse();
  printf("\nSolution vector ordered by solver\n");
  for (int i = 0; i < nc; i++)
  {
    varsFromSCIP[i] = SCIPgetSolVal(scip, SCIPgetBestSol(scip), SCIPgetProbData(scip)->vars[i]);
    if (vars[i] != 0)
      printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
  }
  delete[] varsFromSCIP;

  return 0;
}
