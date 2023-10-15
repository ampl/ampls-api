#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <exception>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR


template <class T> void example()
{
  const char* MODELNAME = "tspg96.nl";
  std::string md(MODELS_DIR);
  md += MODELNAME;
  try {
    T m=ampls::AMPLModel::load<T>(md.c_str());
    m.setOption("nonvalid", 1);
    m.optimize();
    double obj = m.getObj();
    printf("\nObjective (%s)=%f\n", m.driver(), obj);
  }
  catch (const ampls::AMPLSolverException& e)
  {
    // Catch error message 
    std::cout << e.what() << std::endl;
  }
}


int main(int argc, char** argv) {


#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif

#ifdef USE_cplex
  example<ampls::CPLEXModel>();
#endif

#ifdef USE_gurobi
  example<ampls::GurobiModel>();
#endif

#ifdef USE_cbcmp
  example<ampls::CbcModel>();
#endif

#ifdef USE_copt
  example<ampls::CoptModel>();
#endif
#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif
  return 0;
}
