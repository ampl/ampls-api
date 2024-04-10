#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <string>  
#include <cassert>


// This example illustrates how to obtain basic information
// during the solution process using generic callbacks.
const double DESIREDGAP = 0.4;

class MyGenericCallback : public ampls::GenericCallback
{
public:
  double lastBound = std::numeric_limits<double>::infinity();

  int  evaluateObjs(bool mipsol) {
    double objBound = getValue(ampls::Value::MIP_OBJBOUND).dbl;
    double gap = getValue(ampls::Value::MIP_RELATIVEGAP).dbl;
    if (!mipsol) // print only if bound has improved
    {
      if (lastBound == objBound)
        return 0;
    }
    lastBound = objBound;
    printf("Objective: %f6.3 - Bound: %6.3f - RelGap: %6.3f%%\n",
      getObj(), objBound, 100*gap);
    if (gap < DESIREDGAP)
    {
      printf("Desired gap reached, terminating");
      return -1;
    }
    return 0;
  }

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
    //  if (!canDo(ampls::CanDo::GET_MIP_SOLUTION)) return 0; // For some solvers (notably SCIP)
      return evaluateObjs(false);
    case ampls::Where::MIPSOL:
     // if (!canDo(ampls::CanDo::GET_MIP_SOLUTION)) return 0; // For some solvers (notably SCIP)
      double mipgap = getValue(ampls::Value::MIP_RELATIVEGAP).dbl;
      return evaluateObjs(true);
    }
    return 0;
  }

};

template<class T> void example() 
{
  const char* MODELNAME = "queens18.nl";
  std::string md(MODELS_DIR);
  md += MODELNAME;

  T m = ampls::AMPLModel::load<T>(md.c_str() );
  // Set a (generic) callback
  MyGenericCallback cb;
  m.setCallback(&cb);
  try {
    m.setOption("threads", 1);
  }
  catch (...) {}
  m.setAMPLParameter(ampls::SolverParams::DBL_MIPGap, 0.001);
  try {
    m.setOption("outlev", 1);
  }
  catch (const std::exception& e) {
    printf(e.what());
  }
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", m.driver(), obj);
  
  assert( (obj>= 158-158*DESIREDGAP) && (obj <= 158 + 158 * DESIREDGAP) );
  ampls::Status::SolStatus s = m.getStatus();
  assert(s == ampls::Status::INTERRUPTED);
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
    case ampls::Status::INTERRUPTED:
      printf("Interrupted.\n");
      break;
    default:
      printf("Status: %d\n", s);
  }

  // Write the AMPL sol file
  m.writeSol();
}
int main(int argc, char** argv) {
#ifdef USE_highs
  example<ampls::HighsModel >();
#endif

#ifdef USE_cplex
  example<ampls::CPLEXModel >();
#endif

#ifdef USE_copt
  example<ampls::CoptModel>();
#endif

#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif
  
#ifdef USE_gurobi
  example<ampls::GurobiModel>();
#endif
  
#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif

#ifdef USE_cbcmp
  example<ampls::CbcModel>();
#endif*/
  return 0;
 
}
