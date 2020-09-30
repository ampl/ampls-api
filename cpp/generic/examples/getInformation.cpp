// Must be included before ASL, which
// is included by cplex_interface and gurobi_interface
#include "gurobi_c.h"
#include "ilcplex/cplex.h"


#include "cplex_interface.h"
#include "cplex_callback.h"


#include "gurobi_interface.h"
#include "gurobi_callback.h"

#include "simpleapi/simpleApi.h"
#include "test-config.h" // for MODELS_DIR
#include <cstring>
#include <cstdio>

const char* MODELNAME = "tsp.nl";
/*
set A;
var scalar >= 0, <= 4;
var x{a in A} >=0;
var y{a in A, b in A}>=0;
maximize z: scalar + sum{a in A} x[a] + sum{a in A, b in A} y[a,b]*0.1;
c{a in A}: x[a] + sum{b in A} y[a,b] <= 42;

data;

set A := 1 2 3 aa bb cc 'a' 'b' 'c' "d" "e" "f" "4" '5' 6 'a a' "a b" 'ab[c]' "de[f]" 'ab"c' "ab'c";
*/
class MyGenericCallback : public ampls::GenericCallback
{
  virtual int run()
  {
    // Prints out the name of the solution phase where the solver is called from
    // (solver specific)
    printf("\nCalled from %s\n", getWhere());

    // Get the generic mapping
    ampls::CBWhere::Where where = getAMPLType();
    switch (where)
    {
    case ampls::CBWhere::msg:
      printf(getMessage());
      return 0;
    case ampls::CBWhere::presolve:
      if((getValue(ampls::CBValue::pre_delrows).integer+
        getValue(ampls::CBValue::pre_delcols).integer+
        getValue(ampls::CBValue::pre_coeffchanged).integer) > 0)
          printf("\nRemoved %i rows and %i columns. %i coefficients changed", 
            getValue(ampls::CBValue::pre_delrows).integer,
            getValue(ampls::CBValue::pre_delcols).integer,
            getValue(ampls::CBValue::pre_coeffchanged).integer);
          return 0;
    case ampls::CBWhere::mip:
    case ampls::CBWhere::mipsol:
    case ampls::CBWhere::mipnode:
      printf("\nMIP Objective = %f", getObjective());
      return 0;
    case ampls::CBWhere::notmapped:
      printf("\nNot mapped! Where: %s", getWhere());

    }
    return 0;
  }

};

double doStuff(ampls::AMPLModel& m, const char *name) 
{
  // Set a (generic) callback
  MyGenericCallback cb;
  m.setCallback(&cb);
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", name, obj);

  ampls::Status::Status s = m.getStatus();
  switch (s)
  {
    case ampls::Status::Optimal:
      printf("Optimal.\n");
      break;
    case ampls::Status::Infeasible:
      printf("Infeasible.\n");
      break;
    case ampls::Status::Unbounded:
      printf("Unbounded.\n");
      break;
    default:
      printf("Status: %d\n", s);
  }
  // Get the solution vector
  std::size_t nr = m.getNumVars();
  std::vector<double> solution(nr);
  m.getSolution(0, nr, solution.data());
  int nnz = 0;
  for (int i = 0; i < nr; i++)
    if (solution[i] != 0) nnz++;
  printf("\nNumber of non zeroes = %d\n", nnz);

  // Write the AMPL sol file
  m.writeSol();
  return obj;
}
int main(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // TODO
  // Mutex on loadModel functions

  // Gurobi generic
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel mg = gurobi.loadModel(buffer);
  doStuff(mg, "gurobi");

  // CPLEX generic
  //ampls::CPLEXDrv cplex;
  //ampls::CPLEXModel c = cplex.loadModel(buffer);
 // doStuff(c, "cplex");
  return 1;
 
}
