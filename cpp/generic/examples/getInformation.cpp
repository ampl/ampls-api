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
/*
class MyCB : public GRBCallback
{
public:
  int run(GurobiModel* model, void* cbdata, int where)
  {
    //    printf("Called callback with where=%i\n", where);
    std::vector<std::string> vars;
    vars.push_back("x[1]");
    vars.push_back("x[2]");
    double coefs[] = { 5.6, 7.8 };
    int len;
    if (where == GRB_CB_MESSAGE)
    {
      std::string s = get(GRB_CB_MSG_STRING).str;
      // printf("%s\n", s.data());

    }
    else if (where == GRB_CB_PRESOLVE)
    {
      int cdels = get(GRB_CB_PRE_COLDEL).integer;
      int rdels = get(GRB_CB_PRE_ROWDEL).integer;
      printf("%d columns and %d rows are removed\n", cdels, rdels);
    }
    else if (where == GRB_CB_MIP)
    {
      printf("GRB_CB_MIP_SOLCNT %d\n", get(GRB_CB_MIP_SOLCNT).integer);
      printf("GRB_CB_MIP_OBJBST %f\n", get(GRB_CB_MIP_OBJBST).dbl);
      return 0;
    }
    else if ((where == GRB_CB_MIPNODE) ||
      (where == GRB_CB_MIPSOL))
    {

      int len;
      double* sol = getSolutionVector(&len);
      delete[] sol;
      return 0;
      return addCut(vars, coefs, '>', 7);
    }
    else
    {
      printf("Called callback with where=%i\n", where);
    }
    return 0;
  }
};

*/

class CCB : public GenericCallback
{
  virtual int run(int whereFrom)
  {
    printf("Called from %s\n", getWhere(whereFrom));
    switch (getAMPLType())
    {
    case AMPLCBWhere::msg:
      printf("**%s**\n", getMessage());
      return 0;
    case AMPLCBWhere::mipsol:
    case AMPLCBWhere::mipnode:
      printf("MIPSOL OBJ = %f\n", getObjective());
    }
    return 0;
  }

};

double doStuff(AMPLModel& m, const char *name) 
{
  CCB b;
  m.setGenericCallback(&b);
  m.optimize();

  double obj = m.getObj();
  printf("Solution with optimizer %s=%f\n", name, obj);
  return obj;
}
int main(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);
  
  //GurobiDrv gurobi;
  CPLEXDrv cplex;
  //GurobiModel m = gurobi.loadModel(buffer);
  CPLEXModel c = cplex.loadModel(buffer);
  //doStuff(m, "gurobi");
 doStuff(c, "cplex");
}
