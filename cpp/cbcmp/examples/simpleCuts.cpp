#include "cbcmp_interface.h"

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#define CBC_EXTERN_C
#include "CbcModel.hpp"
#include "CbcSolver.hpp"
#include "Cbc_C_Interface.h"


#include <cstring> // for strcat 

class MyCbcCallback : public ampls::CbcCallback
{
public:
  MyCbcCallback() {}

  virtual int run()
  {
    printf("INNER!");
    /*int where = getWhere();
    printf("Where is %d\n", where);
    void*cbdata = getCBData();
    if (where == GRB_CB_MIP) { 
      double nodecount;
      int error = GRBcbget(cbdata, where, GRB_CB_MIP_NODCNT, (void *) &nodecount);
      if (error) return 0;
      printf("MIP node count is %d\n", nodecount);
    }
    */
    return 0;
  }

};

int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "queens18.nl");
  // Load a model using cbc driver
  ampls::CbcDrv cbc;
  ampls::CbcModel m = cbc.loadModel(buffer);

    // Set a (generic) callback
  MyCbcCallback cb;
  m.setCallback(&cb);
  
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("Objective = %f\n", obj);

  // Write the solution back to AMPL
  m.writeSol();
  return 0;
}
