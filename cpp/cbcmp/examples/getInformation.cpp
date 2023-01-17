#include "cbcmp_interface.h"
#include "ampls/ampls.h"


#define CBC_EXTERN_C
#include "CbcModel.hpp"
#include "CbcSolver.hpp"
#include "Cbc_C_Interface.h"

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 
#include <cmath>

const char* MODELNAME = "tsp.nl";

class MyCbcCallback : public ampls::CbcCallback
{
  
  public:
  int lastIter;
  MyCbcCallback() : lastIter(0) { }
  virtual int run()
  {
    printf("called");
    /*
    // Get the data used in a cbc C native callback
    int where = getWhere();
    void* cbdata = getCBData();

    if (where == GRB_CB_MESSAGE)
    {
      // Using ampls cbc-specific get method
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

      // Use cbc functions
       GRBcbget(cbdata, where, GRB_CB_PRE_COLDEL, &cdels);
       GRBcbget(cbdata, where, GRB_CB_PRE_ROWDEL, &rdels);
       if (cdels || rdels)
        printf("Presolve (using C routines): %d columns and %d rows are removed\n", cdels, rdels);

    }
    else if (where == GRB_CB_MIP)
    {
      printf("GRB_CB_MIP_SOLCNT %d\n", getInt(GRB_CB_MIP_SOLCNT));
      printf("GRB_CB_MIP_OBJBST %f\n", getDouble(GRB_CB_MIP_OBJBST));
      double objBest = getDouble(GRB_CB_MIP_OBJBST);
      double objBnd = getDouble(GRB_CB_MIP_OBJBND);
      if (fabs(objBest - objBnd) < 0.1 * (1.0 + fabs(objBest))) {
        printf("Stop early - 10%% gap achieved\n");
        GRBterminate(getGRBModel());
      }
    }
      else if (where == GRB_CB_SIMPLEX)
      {
        // Simplex callback
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
    printf("** End of callback handler **\n\n");
    return 0;
    }
    */
    return 0;
  }
};


void doStuff()
{
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // Load a model using cbc driver
  ampls::CbcDrv cbc;
  ampls::CbcModel model = cbc.loadModel(buffer);

  auto m = model.getCBCmodel();

  Cbc_setParameter(m, "heuristicsOnOff", "off");
  //Cbc_setParameter(m, "greedyHeuristic", "off");
  
  // Create cbc callback and set it using ampls interface
  MyCbcCallback cb;
  model.setCallback(&cb);
  model.optimize();

  // Access objective function through generic API
//  double obj = model.getObj();
 // printf("Objective: %f\n", obj);
}
int main(int argc, char** argv) {
  try {
    doStuff();
  }
  catch (const std::exception& e) {
    printf(e.what());
  }
  return 0;

}
