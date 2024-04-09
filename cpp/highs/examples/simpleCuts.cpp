#include "highs_interface.h"

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "highs.h" // For highs routines
#include <cstring> // for strcat 

class MyHighsCallback : public ampls::HighsCallback
{
public:
  MyHighsCallback() {}

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
  // Load a model using highs driver
  ampls::HighsDrv highs;
  ampls::HighsModel m = highs.loadModel(buffer);

    // Set a (generic) callback
  MyHighsCallback cb;
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
