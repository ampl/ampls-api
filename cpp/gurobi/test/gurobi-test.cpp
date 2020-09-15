#include "gurobi_interface.h"
#include "gurobi_callback.h"

#include "test-config.h" // for MODELS_DIR
#include <cstring>

const char* MODELNAME = "tsp.nl";

class MyCB : public ampl::GRBCallback
{
  int lastIter = 0;
public:
  int run(int where)
  {
    std::vector<std::string> vars;
    vars.push_back("x[1]");
    vars.push_back("x[2]");
    double coefs[] = { 5.6, 7.8 };
    if (where == GRB_CB_MESSAGE)
    {
      std::string s = get(GRB_CB_MSG_STRING).str;
      printf("%s\n", s.data());

    }
    else if (where == GRB_CB_PRESOLVE)
    {
      int cdels = get(GRB_CB_PRE_COLDEL).integer;
      int rdels = get(GRB_CB_PRE_ROWDEL).integer;
      if (cdels || rdels)
        printf("%d columns and %d rows are removed\n", cdels, rdels);
    }
    else if (where == GRB_CB_MIP)
    {
      printf("GRB_CB_MIP_SOLCNT %d\n", get(GRB_CB_MIP_SOLCNT).integer);
      printf("GRB_CB_MIP_OBJBST %f\n", get(GRB_CB_MIP_OBJBST).dbl);
      return 0;
    }
    else if (where == GRB_CB_SIMPLEX)
    {
      /* Simplex callback */
      double itcnt, obj, pinf, dinf;
      int    ispert;
      char   ch;
      itcnt = getInt(GRB_CB_SPX_ITRCNT);
      
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
    else if (where == GRB_CB_MIP)
    {

      double objBest = getDouble(GRB_CB_MIP_OBJBST);
      double objBnd = getDouble(GRB_CB_MIP_OBJBND);
      if (fabs(objBest - objBnd) < 0.1 * (1.0 + fabs(objBest))) {
        printf("Stop early - 10%% gap achieved\n");
        GRBterminate(((ampl::GurobiModel*)this->model_)->getGRBmodel());
      }
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

  // Access objective function through generic API
  double obj = m.getObj();
  printf("Objective: %f\n", obj);

  // Shortcut to access gurobi attribute
  obj = m.getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  printf("Objective from shortcut: %f\n", obj);

  // Gurobi-c way to access attributes
  GRBmodel* grbm = m.getGRBmodel();
  GRBgetdblattr(grbm, GRB_DBL_ATTR_OBJ, &obj);
  printf("Objective from gurobi: %f\n", obj);

  auto fg = m.getVarMap();
  int nv;
  double* vars = m.getSolutionVector(&nv);
  for (auto r : fg)
    printf("Index: %i AMPL: %s=%f\n", r.second, r.first.data(), vars[r.second]);

  auto gf = m.getVarMapInverse();
  double* varsFromGurobi = new double[nv];
  GRBgetdblattrarray(grbm, GRB_DBL_ATTR_X, 0, nv, varsFromGurobi);
  for(int i=0; i<nv;i++)
    printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
    
  delete[] vars;
  delete[] varsFromGurobi;
}
