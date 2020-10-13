#include "gurobi_interface.h"

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "gurobi_c.h" // For gurobi routines
#include <cstring> // for strcat 

class MyGurobiCallback : public ampls::GurobiCallback
{
public:
  MyGurobiCallback() {}

  virtual int run()
  {
    int where = getWhere();
    void*cbdata = getCBData();
    if (where == GRB_CB_MIP) {
      double nodecount;
      int error = GRBcbget(cbdata, where, GRB_CB_MIP_NODCNT, (void *) &nodecount);
      if (error) return 0;
      printf("MIP node count is %d\n", nodecount);
    }
    return 0;
  }

};

int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "queens18.nl");
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel m = gurobi.loadModel(buffer);
    // Set a (generic) callback
  MyGurobiCallback cb;
  m.setCallback(&cb);
  
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("Objective = %f\n", obj);

  // Write the solution back to AMPL
  m.writeSol();
  return obj;


 
}
