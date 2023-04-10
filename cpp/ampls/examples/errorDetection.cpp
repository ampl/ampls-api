#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <exception>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR


template<class T> T loadModel(const char* model, std::vector<std::string> options);

#ifdef USE_gurobi
template<> ampls::GurobiModel loadModel<ampls::GurobiModel>(const char* model, std::vector<std::string> options)
{
  ampls::GurobiDrv grb;
  grb.setOptions(options);
  return grb.loadModel(model);
}
#endif
#ifdef USE_cplexmp
template<> ampls::CPLEXModel loadModel< ampls::CPLEXModel>(const char* model, std::vector<std::string> options)
{
  ampls::CPLEXDrv cplex;
  cplex.setOptions(options);
  return cplex.loadModel(model);
}
#endif
#ifdef USE_xpress
template<> ampls::XPRESSModel loadModel<ampls::XPRESSModel>(const char* model, std::vector<std::string> options)
{
  ampls::XPRESSDrv xpress;
  xpress.setOptions(options);
  return xpress.loadModel(model);
}
#endif

#ifdef USE_cbcmp
template<> ampls::CbcModel loadModel<ampls::CbcModel>(const char* model, std::vector<std::string> options)
{
  ampls::CbcDrv cbc;
  cbc.setOptions(options);
  return cbc.loadModel(model);
}
#endif

#ifdef USE_copt
template<> ampls::CoptModel loadModel<ampls::CoptModel >(const char* model, std::vector<std::string> options)
{
  ampls::CoptDrv copt;
  copt.setOptions(options);
  return copt.loadModel(model);
}
#endif
template <class T> double doStuff(const char* model)
{
  try {
    T m= loadModel<T>(model, {"wrongopt=1"});
    m.optimize();
    double obj = m.getObj();
    printf("\nObjective (%s)=%f\n", m.driver(), obj);
    return obj;
  }
  catch (const ampls::AMPLSolverException& e)
  {
    // Catch error message (arising from the inner driver, 
    // since the bad option is detected by ASL's getopts)
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

#ifdef USE_xgurobi
  doStuff<ampls::XGurobiModel>(buffer);
#endif

}
