#include <string>
#include <exception>
#include <cassert>
#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"

void makeAmplModel(ampl::AMPL &ampl , int numvars, bool unfeasible, bool flipData = false) {
  ampl.eval("set varIndexes within Integers; set constraintIndexes within Integers;"
    "param varCosts{ varIndexes };"
    "param rightHandSides{ constraintIndexes };"
    "var x{ i in varIndexes } >= 0;"
    "maximize objectiveFunction : sum{ i in varIndexes } varCosts[i] * x[i];"
    " subject to mainConstraints{ j in constraintIndexes } : x[j] + x[j + 1] <= rightHandSides[j];");

  ampl::DataFrame d(1, { "varIndexes", "varCosts" });
  for (int i = 0; i < numvars; i++)
    d.addRow(i, flipData ? -i : i);
  ampl.setData(d, "varIndexes");

  ampl::DataFrame d2(1, { "constraintIndexes", "rightHandSides" });
  for (int i = 0; i < numvars - 1; i++)
    d2.addRow(i, unfeasible ? -1 : 1);
  ampl.setData(d2, "constraintIndexes");
}
template <class T> T solveModel(ampl::AMPL& ampl) {
    auto model = ampls::AMPLAPIInterface::exportModel<T>(ampl);
    //gurobiModel.setOption("mip_gap", 1);
    model.optimize();
    ampls::AMPLAPIInterface::importModel(ampl, model);
    return model;
}


template <class T> void createAndSolveSimpleModel() {
  int numVars = 10;
  ampl::AMPL ampl;
  makeAmplModel(ampl, numVars, false);
  auto model = solveModel<T>(ampl);
  //int statusNum = gurobiModel.getIntAttr("Status");
  //./assert(gurobiModel.getStatus() == ampls::Status::OPTIMAL);
  //assert(statusNum == GRB_OPTIMAL);
  //#Num Vars is even->last var idx is odd->expect odd idx vars to be 1
  //#Num Vars is odd->last var idx is even->expect even idx vars to be 1
  std::vector<double> expectedSolution(numVars);
  double expectedObjective = 0;
  for (int i = 0; i < numVars; i++) {
    expectedSolution[i] = std::abs(i % 2 - numVars % 2);
    expectedObjective += i * expectedSolution[i];
  }
  double solverObjective = model.getObj();
  assert(solverObjective == expectedObjective);
  printf("Completed Simple Model Test.\n\n");
}


template <class T> void createAndSolveInfeasibleModel(bool presolve) {
    int numVars = 10;
    ampl::AMPL ampl;
    makeAmplModel(ampl, numVars, true);
    ampl.setIntOption("presolve", presolve ? 10 : 0); // 10 is the default number of presolve passes
    auto model = solveModel<T>(ampl);
    
    assert(model.getStatus() == ampls::Status::INFEASIBLE);
    //int statusNum = gurobiModel.getIntAttr("Status");
    //assert(statusNum == GRB_INFEASIBLE);
    printf("Completed Infeasible Model Test\n\n");
}

template <class T> void run() {
  // This will fail because AMPL's presolve will block the 
// export of the model
  try {
    createAndSolveInfeasibleModel<T>(true);
  }
  catch (const ampls::AMPLSolverException& e) {
    printf("Caught exception: %s\n\n", e.what());
  }

  createAndSolveInfeasibleModel<T>(false);
  createAndSolveSimpleModel<T>();
}

int main(int argc, char** argv) {

#ifdef USE_gurobi
  run<ampls::GurobiModel>();
#endif

#ifdef USE_cbcmp
  run<ampls::CbcModel>();
#endif

#ifdef USE_copt
  run<ampls::CoptModel>();
#endif

#ifdef USE_cplex
  run<ampls::CPLEXModel>();
#endif

#ifdef USE_xpress
  run<ampls::XPRESSModel>();
#endif
}