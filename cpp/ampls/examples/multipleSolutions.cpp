#include <string>
#include <exception>
#include <cassert>
#include <iostream> // for ostringstream

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"

void createModel(ampl::AMPL& ampl) {
  ampl.eval("var x binary; var y binary; var z binary;"
    "minimize TotalSum: z + 1;"
    "subj to C1 : x + y >= 1;");
}

template <class T> T solveModel(ampl::AMPL& ampl) {
    T model = ampls::AMPLAPIInterface::exportModel<T>(ampl);
    model.setOption("sol:stub", "stub");
    model.setOption("sol:poolgap", 0.1);
    model.setOption("outlev", 1);

    // Have to refresh to make the driver aware of the options
    model.refresh();
    // Use AMPLModel::optimize() so that the driver "solve" function
    // is used
    model.optimize();
    return model;
}

int main(int argc, char** argv) {
  ampl::AMPL ampl;
  createModel(ampl);
  auto m = solveModel<ampls::GurobiModel>(ampl);
  ampls::AMPLAPIInterface::importModel(ampl, m);
  
  int nsol = static_cast<int>(ampl.getValue("TotalSum.nsol").dbl());
  printf("Gotten %d solutions\n", nsol);

  // Load solutions into the AMPL object and display them
  char BUFFER[24];
  for (int i = 1; i <= nsol; i++) {
    sprintf(BUFFER, "solution stub%i.sol;", i);
    ampl.eval(BUFFER);
    std::cout << ampl.getData("x,y,z").toString();
  }

}
