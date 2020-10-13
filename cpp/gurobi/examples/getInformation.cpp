#include "gurobi_interface.h"
#include "ampls/ampls.h"

#include "gurobi_c.h"

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 
#include <cmath>

const char* MODELNAME = "tsp.nl";

class MyGurobiCallback : public ampls::GurobiCallback
{
  public:
  int lastIter;
  MyGurobiCallback() : lastIter(0) { }
  virtual int run()
  {
    // Get the data used in a gurobi C native callback
    int where = getWhere();
    void* cbdata = getCBData();

    if (where == GRB_CB_MESSAGE)
    {
      // Using ampls gurobi-specific get method 
      std::string s = get(GRB_CB_MSG_STRING).str;
      printf("%s", s.data());
      return 0;
    }
    if (where == GRB_CB_PRESOLVE)
    {
      // Use ampls shortcuts
      int cdels = getInt(GRB_CB_PRE_COLDEL);
      int rdels = getInt(GRB_CB_PRE_ROWDEL);
      if (cdels || rdels)
        printf("Presolve: %d columns and %d rows are removed\n", cdels, rdels);

      // Use gurobi functions
       GRBcbget(cbdata, where, GRB_CB_PRE_COLDEL, &cdels);
       GRBcbget(cbdata, where, GRB_CB_PRE_ROWDEL, &rdels);
       if (cdels || rdels)
        printf("Presolve (using C routines): %d columns and %d rows are removed\n", cdels, rdels);

    }
    else if (where == GRB_CB_MIP)
    {
      printf("GRB_CB_MIP_SOLCNT %d\n", getInt(GRB_CB_MIP_SOLCNT));
      printf("GRB_CB_MIP_OBJBST %f\n", getDouble(GRB_CB_MIP_OBJBST));
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
        GRBterminate(getGRBModel());
      }
    }
    printf("** End of callback handler **\n\n");
    return 0;
  }
};


int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel model = gurobi.loadModel(buffer);
  
  // Create gurobi callback and set it using ampls interface
  MyGurobiCallback cb;
  model.setCallback(&cb);
  GRBmodel* grb=  model.getGRBmodel();

  // Start optimization gurobi's functions 
  GRBoptimize(grb);
  
  // Access objective function through generic API
  double obj = model.getObj();
  printf("Objective: %f\n", obj);
  return 0;
}
