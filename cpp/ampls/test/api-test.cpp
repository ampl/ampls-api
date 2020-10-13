#ifdef USE_cplex
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_xpress
#include "xpress_interface.h"
#endif

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstring>

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

class CCB : public ampls::GenericCallback
{
  virtual int run()
  {
    // printf("Called from %s\n", getWhere(whereFrom));
    switch (getAMPLWhere())
    {
    case ampls::Where::MSG:
  //    printf("**%s**\n", getMessage());
      return 0;
    case ampls::Where::MIPSOL:
    case ampls::Where::MIPNODE:
      try {
        printf("MIPSOL OBJ = %f\n", getObj());
      }
      catch (const ampls::AMPLSolverException& s)
      {
        printf("%s", s.what());
      }
    }
    return 0;
  }

};

double doStuff(ampls::AMPLModel& m, const char *name)
{
  CCB b;
  m.setCallback(&b);
  m.optimize();
  m.getVarMap();
  double obj = m.getObj();
  printf("Solution with optimizer %s=%f\n", name, obj);
  return obj;
}
int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);
#ifdef USE_cplex
  // Load a model using CPLEX driver
  ampls::CPLEXDrv cplex;
  ampls::CPLEXModel c = cplex.loadModel(buffer);
  // Use it as generic model
  doStuff(c, "cplex");
#endif
#ifdef USE_gurobi
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel g = gurobi.loadModel(buffer);
  // Use it as generic model
  doStuff(g, "gurobi");
#endif

#ifdef USE_xpress
  // Load a model using CPLEX driver
  ampls::XPRESSDrv xpress;
  ampls::XPRESSModel x = xpress.loadModel(buffer);
  // Use it as generic model
  doStuff(x, "xpress");
#endif
}
