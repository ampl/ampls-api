#include "gurobi_interface.h"
#include "gurobi_callback.h"

#include "test-config.h" // for MODELS_DIR
#include <cstring>
#include <vector>

#include <cmath>

const char* MODELNAME = "testmodel.nl";

class MyGurobiCutCallback : public ampls::GurobiCallback
{
  int run()
  {
    int ret = 0;
    if (getWhere() == GRB_CB_MESSAGE)
    {
      printf("%s", getMessage());
      return 0;
    }
    if (getWhere() == GRB_CB_MIPSOL)
    {
      std::vector<std::string> vars;
      vars.push_back("x[1,2]");
      vars.push_back("x[1,4]");
      double coefs[] = { 1,1};
      std::vector<double> sol = getSolutionVector();
      for (int i = 0; i < sol.size(); i++)
        if(sol[i]!=0)
          printf("x[%d] = %f\n", i, sol[i]);
      auto c= addLazy(vars, coefs, ampls::CutDirection::GE, 2);
    }
    return 0;
  }
};
class MyGurobiCallback : public ampls::GurobiCallback
{
  int lastIter = 0;
public:
  int run()
  {
    if (getWhere() == GRB_CB_MESSAGE)
    {
      std::string s = get(GRB_CB_MSG_STRING).str;
      printf("%s", s.data());
      return 0;
    }
    printf("\n** Called callback with where=%s ** \n", getWhereString());
    if (getWhere() == GRB_CB_PRESOLVE)
    {
      int cdels = get(GRB_CB_PRE_COLDEL).integer;
      int rdels = get(GRB_CB_PRE_ROWDEL).integer;
      if (cdels || rdels)
        printf("%d columns and %d rows are removed\n", cdels, rdels);
    }
    else if (getWhere() == GRB_CB_MIP)
    {
      printf("GRB_CB_MIP_SOLCNT %d\n", get(GRB_CB_MIP_SOLCNT).integer);
      printf("GRB_CB_MIP_OBJBST %f\n", get(GRB_CB_MIP_OBJBST).dbl);
    }
    else if (getWhere() == GRB_CB_SIMPLEX)
    {
      /* Simplex callback */
      double itcnt, obj, pinf, dinf;
      int    ispert;
      char   ch;
      itcnt = getDouble(GRB_CB_SPX_ITRCNT);
      
      if (itcnt - lastIter >= 100) {
        lastIter = itcnt;
        obj = getDouble(GRB_CB_SPX_OBJVAL);
        ispert = getInt(GRB_CB_SPX_ISPERT);
        pinf = getDouble(GRB_CB_SPX_PRIMINF);
        dinf = getDouble(GRB_CB_SPX_DUALINF);
        if (ispert == 0) ch = ' ';
        else if (ispert == 1) ch = 'S';
        else                  ch = 'P';
        printf("%7.0f %14.7e%c %13.6e %13.6e\n", itcnt, obj, ch, pinf, dinf);
      }
    }
    else if (getWhere() == GRB_CB_MIP)
    {
      double objBest = getDouble(GRB_CB_MIP_OBJBST);
      double objBnd = getDouble(GRB_CB_MIP_OBJBND);
      if (fabs(objBest - objBnd) < 0.1 * (1.0 + fabs(objBest))) {
        printf("Stop early - 10%% gap achieved\n");
        GRBterminate(((ampls::GurobiModel*)this->model_)->getGRBmodel());
      }
    }
    printf("** End of callback handler **\n\n");
    return 0;
  }
};

int main(int argc, char** argv) {

  int res = 0;

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  ampls::GurobiDrv d;
  ampls::GurobiModel m = d.loadModel(buffer);
  
  auto options = m.getOptions();
  for (auto o : options)
    printf("%s\n", o.toString().c_str());

  m.setOption("mip:return_gap", 1);
  printf("Option=%d\n", m.getIntOption("mip:return_gap"));
  m.enableLazyConstraints();
  MyGurobiCallback cb;
  res = m.setCallback(&cb);
  if (res != 0)
  {
    printf("ERROR!!! %i\n", res);
    return res;
  }
  
  m.optimize();

  // Access objective function through generic API
  double obj = m.getObj();
  printf("Objective: %f\n", obj);

  // Access gurobi attribute with shortcut function getDoubleAttr
  obj = m.getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  printf("Objective from shortcut: %f\n", obj);

  // Use Gurobi C API to access attributes
  GRBmodel* grbm = m.getGRBmodel();
  GRBgetdblattr(grbm, GRB_DBL_ATTR_OBJ, &obj);
  printf("Objective from gurobi: %f\n", obj);
  m.writeSol();
  // Get solution vector via generic interface
  std::vector<double> vars = m.getSolutionVector();
  // Get map and display the variables ordered as AMPL stores them
  auto fg = m.getVarMap();
  double value;
  printf("\nSolution vector ordered by AMPL definition\n");
  for (auto r : fg)
  {
    value = vars[r.second];
    if (value != 0)
      printf("Index: %i AMPL: %s=%f\n", r.second, r.first.data(), value);
  }
  
  // Get solution vector via Gurobi C API
  int nc;
  GRBgetintattr(grbm, GRB_INT_ATTR_NUMVARS, &nc);
  double* varsFromGurobi = new double[nc];
  GRBgetdblattrarray(grbm, GRB_DBL_ATTR_X, 0, nc, varsFromGurobi);

  // Get inverse map and display the variables with solver ordering
  auto gf = m.getVarMapInverse();
  printf("\nSolution vector ordered by solver\n");
  for (int i = 0; i < nc; i++)
  {
    if (vars[i] != 0)
      printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
  }
  delete[] varsFromGurobi;
}
