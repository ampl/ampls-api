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
class MyGenericCallback : public GenericCallback
{
  virtual int run(int whereFrom)
  {
    //printf("Called from \n", getWhere(whereFrom));
    switch (getAMPLType())
    {
    case AMPLCBWhere::msg:
    //  printf("**%s**\n", getMessage());
      return 0;
    case AMPLCBWhere::presolve:
      printf("Removed %i rows and %i columns. %i coefficients changed\n", 
        this->getValue(AMPLCBValue::pre_delrows).integer,
        this->getValue(AMPLCBValue::pre_delcols).integer,
        this->getValue(AMPLCBValue::pre_coeffchanged).integer
        );
      return 0;
    case AMPLCBWhere::mip:
    case AMPLCBWhere::mipsol:
    case AMPLCBWhere::mipnode:
      printf("MIP OBJ = %f\n", getObjective());
      return 0;
    case AMPLCBWhere::notmapped:
      printf("Not mapped! Where: %s\n", getWhere(whereFrom));

    }
    return 0;
  }

};

double doStuff(AMPLModel& m, const char *name) 
{
  MyGenericCallback cb;
  m.setGenericCallback(&cb);
  m.optimize();

  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", name, obj);
  return obj;
}
int main(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // Gurobi generic
  GurobiDrv gurobi;
  GurobiModel mg = gurobi.loadModel(buffer);
  doStuff(mg, "gurobi");

  // CPLEX generic
  CPLEXDrv cplex;
  CPLEXModel c = cplex.loadModel(buffer);
  doStuff(c, "cplex");

 
}
