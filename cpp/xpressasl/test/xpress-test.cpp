#include "xpress_interface.h"

#include "test-config.h" // for MODELS_DIR

#include <cstring>

const char* MODELNAME = "tsp.nl";


int main(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);
  int res = 0;
  ampls::XPRESSDrv d;
  try {
    ampls::XPRESSModel m = d.loadModel(buffer);
    m.optimize();
    double obj = m.getObj();
    m.writeSol();
    m.printModelVars(true);
  }
  catch (const std::exception& e)
  {
    printf("%s", e.what());
  }
}
