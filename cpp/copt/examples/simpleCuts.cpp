#include "copt_interface.h"

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "copt.h" // For copt routines
#include <cstring> // for strcat 

class MyCoptCallback : public ampls::CoptCallback
{
public:
  MyCoptCallback() {}

  virtual int run()
  {
    int where = getWhere();
    printf("Where is %d\n", where);
    void*cbdata = getCBData();
    return 0;
  }

};

int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "queens18.nl");
  // Load a model using copt driver
  ampls::CoptDrv copt;
  ampls::CoptModel m = copt.loadModel(buffer);

    // Set a (generic) callback
  MyCoptCallback cb;
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
