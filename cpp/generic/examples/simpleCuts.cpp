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
class MyGenericCallback : public ampl::GenericCallback
{
  virtual int run(int whereFrom)
  {
    // Prints out the name of the solution phase where the solver is called from
    // (solver specific)
    printf("Called from %s\n", getWhere(whereFrom));

    // Get the generic mapping
    ampl::AMPLCBWhere::Where where = getAMPLType();
    switch (where)
    {
    case ampl::AMPLCBWhere::msg:
    //  printf("**%s**\n", getMessage());
      return 0;
    case ampl::AMPLCBWhere::presolve:
      if((getValue(ampl::AMPLCBValue::pre_delrows).integer+
        getValue(ampl::AMPLCBValue::pre_delcols).integer+
        getValue(ampl::AMPLCBValue::pre_coeffchanged).integer) > 0)
          printf("Removed %i rows and %i columns. %i coefficients changed\n", 
            getValue(ampl::AMPLCBValue::pre_delrows).integer,
            getValue(ampl::AMPLCBValue::pre_delcols).integer,
            getValue(ampl::AMPLCBValue::pre_coeffchanged).integer);
          return 0;
    case ampl::AMPLCBWhere::mip:
    case ampl::AMPLCBWhere::mipsol:
    case ampl::AMPLCBWhere::mipnode:
      printf("MIP Objective = %f\n", getObjective());
      return 0;
    case ampl::AMPLCBWhere::notmapped:
      printf("Not mapped! Where: %s\n", getWhere(whereFrom));

    }
    return 0;
  }

};

double doStuff2(ampl::AMPLModel& m, const char *name) 
{
  // Set a (generic) callback
  MyGenericCallback cb;
  m.setGenericCallback(&cb);
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", name, obj);
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
void main2(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "tsp.nl");


 
}
