#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <exception>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR


template <class T> double doStuff(const char* model)
{
  try {
    T m=ampls::AMPLModel::load<T>(model);
    m.setOption("nonvalid", 1);
    m.optimize();
    double obj = m.getObj();
    printf("\nObjective (%s)=%f\n", m.driver(), obj);
    return obj;
  }
  catch (const ampls::AMPLSolverException& e)
  {
    // Catch error message 
    std::cout << e.what() << std::endl;
  }
}


int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "tspg96.nl");

#ifdef USE_xpressmp
  doStuff<ampls::XPRESSModel>(buffer);
#endif

#ifdef USE_cplexmp
  doStuff<ampls::CPLEXModel>(buffer);
#endif

#ifdef USE_gurobi
  doStuff<ampls::GurobiModel>(buffer);
#endif

#ifdef USE_cbcmp
  doStuff<ampls::CbcModel>(buffer);
#endif

#ifdef USE_copt
  doStuff<ampls::CoptModel>(buffer);
#endif

}
