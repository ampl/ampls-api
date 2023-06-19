#include "scip_interface.h"
#include "ampls/ampls.h"

#include "scip/scip.h" 

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "testmodel.nl";

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
  //std::vector<double> vars = model.getSolutionVector();
  // Get map and display the variables ordered as AMPL stores them
  //auto fg = model.getVarMap();
  //double value;
  //printf("\nSolution vector ordered by AMPL definition\n");
  //for (auto r : fg)
  //{
  //  value = vars[r.second];
  //  if (value != 0)
  //    printf("Index: %i AMPL: %s=%f\n", r.second, r.first.data(), value);
  //}
  
  // Get solution vector via Gurobi C API
  //int nc;
  //GRBgetintattr(grb, GRB_INT_ATTR_NUMVARS, &nc);
  //double* varsFromGurobi = new double[nc];
  //GRBgetdblattrarray(grb, GRB_DBL_ATTR_X, 0, nc, varsFromGurobi);

  // Get inverse map and display the variables with solver ordering
  //auto gf = model.getVarMapInverse();
  //printf("\nSolution vector ordered by solver\n");
  //for (int i = 0; i < nc; i++)
  //{
  //  if (vars[i] != 0)
  //    printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
  //}
  //delete[] varsFromGurobi;

  return 0;
}
