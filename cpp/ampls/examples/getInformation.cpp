#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "queens18.nl";

// This example illustrates how to obtain basic information
// during the solution process using generic callbacks.

class MyGenericCallback : public ampls::GenericCallback
{
  int nMIPnodes = 0;
  int nadd = 0;
  int run()
  {
    // Prints out the name of the solution phase where the solver is called from
    // (solver specific)
    //printf("\nCalled from %s\n", getWhere());
    double obj;
    // Get the generic mapping
     ampls::Where::CBWhere where = getAMPLWhere();
     //printf("Where: %i\n", where);
    switch (where)
    {
    case ampls::Where::MSG:
      // Print out all messages coming from the solver
      std::cout<<getMessage();
      return 0;
    case ampls::Where::PRESOLVE:
      if((getValue(ampls::Value::PRE_DELROWS).integer+
        getValue(ampls::Value::PRE_DELCOLS).integer+
        getValue(ampls::Value::PRE_COEFFCHANGED).integer) > 0)
          printf("\nRemoved %i rows and %i columns. %i coefficients changed", 
            getValue(ampls::Value::PRE_DELROWS).integer,
            getValue(ampls::Value::PRE_DELCOLS).integer,
            getValue(ampls::Value::PRE_COEFFCHANGED).integer);
          return 0;
    case ampls::Where::MIPNODE:
      nMIPnodes++;
      //printf("\nNew MIP node. Count: %d", nMIPnodes);
      //printf("\nRel MIP GAP: %f", getValue(ampls::Value::MIP_RELATIVEGAP).dbl);
      return 0;
    case ampls::Where::MIP:
    case ampls::Where::MIPSOL:
      try {
        obj = getObj();
        printf("\nMIP Objective = %f", getObj());
        printf("\nRel MIP GAP: %f", getValue(ampls::Value::MIP_RELATIVEGAP).dbl);
        return 0;
      }
      catch (...) {
        return 0;
      }
    }
    return 0;
  }

};

template<class T> double doStuff(const char* nlfile) 
{
  T m = ampls::AMPLModel::load<T>(nlfile);
  // Set a (generic) callback
  MyGenericCallback cb;
  m.setCallback(&cb);
  m.setAMPLParameter(ampls::SolverParams::DBL_MIPGap, 0.001);
  try {
    m.setOption("return_mipgap", 5);
    m.setOption("mipstartvalue", 3);
    m.setOption("mipstartalg", 2);
    m.setOption("mipdisplay", 2);
  }
  catch (const std::exception& e) {
    printf(e.what());
  }
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", m.driver(), obj);

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

  // Write the AMPL sol file
  m.writeSol();
  return obj;
}
int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

#ifdef USE_cplex
  doStuff<ampls::CPLEXModel >(buffer);
#endif
#ifdef USE_gurobi
  //doStuff<ampls::GurobiModel>(buffer);
#endif

#ifdef USE_copt
  doStuff<ampls::CoptModel >(buffer);
#endif


#ifdef USE_xpress
  doStuff<ampls::XPRESSModel >(buffer);
#endif

#ifdef USE_cbcmp
  doStuff<ampls::CbcModel >(buffer);
#endif
  return 0;
 
}
