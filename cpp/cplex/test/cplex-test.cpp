#include "cplex_interface.h"

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
class IBMCB :public ampls::CPLEXCallback 
{
public:
  virtual int 
    run()  {
    int    status = 0;
    int    phase = -1;
    double suminf_or_objective;
    int    itcnt = -1;
   // printf("Called with where: %s\n", getWhere(where_));
    if (where_ == CPX_CALLBACK_MIP_CUT_FEAS)
    {
      std::vector<std::string> vars;
      vars.push_back("x[1,34]");
      vars.push_back("x[10,16]");
      double coefs[] = {1, 1};
    /*  std::vector<double> sol = getSolutionVector();
      for (int i = 0; i < sol.size(); i++)
        printf("x[%d] = %f\n", i, sol[i]);
        */
      status= addLazy(vars, coefs, ampls::CutDirection::GE, 2);
      std::vector<int>indices;
      indices.push_back(1);
      indices.push_back(2);
      //status = addLazyIndices(2, indices.data(), coefs, ampls::CBDirection::ge, 1);
      if (status != 0)
        return status;
    }
    if (where_ == CPX_CALLBACK_PRESOLVE) {

      status = CPXgetcallbackinfo(getCPXENV(), getCBData(), where_,
        CPX_CALLBACK_INFO_PRESOLVE_COLSGONE, &itcnt);
      if (status)  goto TERMINATE;
    //  printf("Eliminated %d columns\n", itcnt);
      return 0;
    }
    if (where_ == CPX_CALLBACK_PRIMAL) {
      status = CPXgetcallbackinfo(getCPXENV(), getCBData(), where_,
        CPX_CALLBACK_INFO_ITCOUNT, &itcnt);
      if (status)  goto TERMINATE;

      status = CPXgetcallbackinfo(getCPXENV(), getCBData(), where_,
        CPX_CALLBACK_INFO_PRIMAL_FEAS, &phase);
      if (status)  goto TERMINATE;

      if (phase == 0) {
        status = CPXgetcallbackinfo(getCPXENV(), getCBData(), where_,
          CPX_CALLBACK_INFO_PRIMAL_INFMEAS,
          &suminf_or_objective);
        if (status)  goto TERMINATE;

        printf("Iteration %d: Infeasibility measure = %f\n",
          itcnt, suminf_or_objective);
      }
      else {
        status = CPXgetcallbackinfo(getCPXENV(), getCBData(), where_,
          CPX_CALLBACK_INFO_PRIMAL_OBJ,
          &suminf_or_objective);
        if (status)  goto TERMINATE;

        printf("Iteration %d: Objective = %f\n",
          itcnt, suminf_or_objective);
      }

    }

  TERMINATE:

    return (status);
  }
};

class MyCB : public ampls::CPLEXCallback
{
public:
  virtual int run(CPXCENVptr env, void* lp, int wf) {
    std::vector<std::string> vars;
    vars.push_back("x[1]");
    vars.push_back("x[2]");
    double coefs[] = { 5.6, 7.8 };
    int len;
    printf("OBJ = %f\n", getObj());
    /*
    // TODO MAP where from 
    if (where == GRB_CB_MESSAGE)
    {
      std::string s = getMessage();
      printf("%s\n", s.data());

    }
    else if ((where == GRB_CB_MIPNODE) ||
      (where == GRB_CB_MIPSOL))
    {
      double* sol = getSolution(&len);
      return addCut(vars, coefs, '>', 7);
    }
    else
    {
      printf("Called callback with where=%i\n", where);
    }
    return 0;
    */
    return 0;
  }
};

int main(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);
  int res = 0;
  ampls::CPLEXDrv d;
  IBMCB cb;
  double obj=0;
  try {
    ampls::CPLEXModel m = d.loadModel(buffer);
   
  
    res = m.setCallback(&cb);
    if (res != 0)
    {
      printf("ERROR!!! %i\n", res);
      return res;
    }

    m.optimize();
    obj = m.getObj();
    m.writeSol();
    m.printModelVars(true);
  }
  catch (const std::exception& e)
  {
    printf(e.what());
  }
  
  

  ampls::CPLEXModel m2 = d.loadModel(buffer);
 // res = m2.setCallback(&cb);
  m2.optimize();
  double obj2 = m2.getObj();
  m2.writeSol();
  m2.printModelVars(true);
  printf("Objectives: %f - %f\n", obj, obj2);
}
