#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 

const char* MODELNAME = "tsp.nl";

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
     ampls::Where::CBWhere where = getAMPLSWhere();
     //printf("Where: %i\n", where);
    
    char BUFFER[100];
    
    switch (where)
    {
    case ampls::Where::MSG:
      std::cout<<getMessage() << std::endl;
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
      printf("\nNew MIP node. Count: %d", nMIPnodes);
      printf("\nRel MIP GAP: %f", getValue(ampls::Value::MIP_RELATIVEGAP).dbl);
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
    case ampls::Where::NOTMAPPED:
      printf("\nNot mapped! Where: %s", getWhereString());
    }
    return 0;
  }

};

double doStuff(ampls::AMPLModel& m, const char *name) 
{
  // Set a (generic) callback
  MyGenericCallback cb;
  m.setCallback(&cb);
  m.setAMPLSParameter(ampls::SolverParams::DBL_MIPGap, 0.001);
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", name, obj);

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
  // Get the solution vector
  std::vector<double> solution = m.getSolutionVector();
  int nnz = 0;
  for (int i = 0; i < solution.size(); i++)
    if (solution[i] != 0) nnz++;
  printf("\nNumber of non zeroes = %d\n", nnz);

  double mipgap = m.getAMPLSDoubleAttribute(ampls::SolverAttributes::DBL_RelMIPGap);
  printf("\nSolution MIP gap=%f\n", mipgap);

  // Write the AMPL sol file
  m.writeSol();
  return obj;
}
int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);
#ifdef USE_gurobi
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel g = gurobi.loadModel(buffer);
  // Use it as generic model
  doStuff(g, "gurobi");
#endif

#ifdef USE_copt
  // Load a model using CPLEX driver
  ampls::CoptDrv copt;
  ampls::CoptModel coptmodel = copt.loadModel(buffer);
  // Use it as generic model
  doStuff(coptmodel, "copt");
#endif

#ifdef USE_cplexmp
  // Load a model using CPLEX driver
  ampls::CPLEXDrv cplex;
  ampls::CPLEXModel c = cplex.loadModel(buffer);
  // Use it as generic model
  doStuff(c, "cplex");
#endif

#ifdef USE_xpress
  // Load a model using Xpress driver
  ampls::XPRESSDrv xpress;
  ampls::XPRESSModel x = xpress.loadModel(buffer);
  // Use it as generic model
  doStuff(x, "xpress");
#endif

#ifdef USE_cbcmp
  // Load a model using CPLEX driver
  ampls::CbcDrv cbc;
  ampls::CbcModel cbcmodel = cbc.loadModel(buffer);
  // Use it as generic model
  doStuff(cbcmodel, "cbc");
#endif


  return 0;
 
}
