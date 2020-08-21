#include "gurobi_interface.h"
#include "gurobi_callback.h"

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

class MyCB : public ampl::GRBCallback
{
public:
  //int run(GurobiModel* model, void* cbdata, int where)
  int run(int where)
  {
//    printf("Called callback with where=%i\n", where);
    std::vector<std::string> vars;
    vars.push_back("x[1]");
    vars.push_back("x[2]");
    double coefs[] = { 5.6, 7.8 };
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

int main(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);
  ampl::GurobiDrv d;
  ampl::GurobiModel m = d.loadModel(buffer);
  MyCB cb;
  int res = 0;
  res = m.setCallback(&cb);
  if (res != 0)
  {
    printf("ERROR!!! %i\n", res);
    return res;
  }

  m.optimize();
  double obj = m.getObj();
  double gg = m.getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  m.writeSol();
  

  ampl::GurobiModel m2 = d.loadModel(buffer);
  m2.optimize();
  double obj2 = m2.getObj();
  m2.writeSol();
  double gg2 = m2.getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  //auto fg = m.getVarMap();
  //for (auto r : fg)
  //  printf("VAR: *%s* Index: %i\n", r.first.data(), r.second);
  printf("Objectives: %f - %f\n", obj, obj2);
  printf("\nAttrib: %f\n", gg);
  printf("nAttrib2: %f\n", gg);
  
}
