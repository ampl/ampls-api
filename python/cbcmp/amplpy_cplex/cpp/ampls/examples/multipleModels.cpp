#include <string>
#include <exception>
#include <cassert>
#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"


/// <summary>
/// Create an AMPL model using the AMPL API
/// </summary>
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

/// <summary>
/// Export the AMPL model to a (templated) AMPLS solver and solve it via AMPLS
/// Reimports the model in the AMPL API object and also returns the AMPLS object for
/// further analysis 
/// </summary>
template <class T> T solveModel(ampl::AMPL& ampl, const std::map<std::string, int>& options = {}) {
    auto model = ampls::AMPLAPIInterface::exportModel<T>(ampl);
    for (auto o : options)
      model.setOption(o.first.c_str(), o.second);
    model.optimize();
    ampls::AMPLAPIInterface::importModel(ampl, model);
    return model;
}
/// <summary>
/// Template function for solver-specific methods
/// </summary>
template <class T> void checkNativeStatus(T& model) {}
#ifdef USE_gurobi
void checkNativeStatus(ampls::GurobiModel& grbmodel) {
  // Use AMPLS wrapper to Gurobi native function
  int status = grbmodel.getIntAttr(GRB_INT_ATTR_STATUS);
  assert(status == GRB_OPTIMAL);

  // Use Gurobi native functions
  GRBgetintattr(grbmodel.getGRBmodel(), GRB_INT_ATTR_STATUS, &status);
  assert(status == GRB_OPTIMAL);
}
#endif
#ifdef USE_cplex
void checkNativeStatus(ampls::CPLEXModel& model) {
  // Use CPLEX native functions
  int status = CPXgetstat(model.getCPXENV(), model.getCPXLP());
  assert(status == CPXMIP_OPTIMAL);
}
#endif
#ifdef USE_xpress
void checkNativeStatus(ampls::XPRESSModel& model) {
  // Use AMPLS wrapper to XPRESS native function
  int status = model.getIntAttr(XPRS_LPSTATUS);
  assert(status = XPRS_LP_OPTIMAL);
  
  // Use XPRESS native functions
  XPRSgetintattrib(model.getXPRSprob(), XPRS_LPSTATUS, &status);
  assert(status = XPRS_LP_OPTIMAL);
}
#endif


template <class T> void createAndSolveSimpleModel() {
  int numVars = 10;
  ampl::AMPL ampl;
  makeAmplModel(ampl, numVars, false);
  auto model = solveModel<T>(ampl);

  // Get status throught AMPLS
  assert(model.getStatus() == ampls::Status::OPTIMAL);

  // Demonstrate solver native methods
  checkNativeStatus(model);

  // Num Vars is even->last var idx is odd->expect odd idx vars to be 1
  // Num Vars is odd->last var idx is even->expect even idx vars to be 1
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
    std::map<std::string, int> options = { {"outlev", 1}, {"iisfind", 1} };
    auto model = solveModel<T>(ampl, options);
    assert(model.getStatus() == ampls::Status::INFEASIBLE);
    printf("Completed Infeasible Model Test\n\n");
}


template <class T> void run() {
  try {
    // This will fail because AMPL's presolve will block the 
    // export of the model
    createAndSolveInfeasibleModel<T>(true);
  }
  catch (const ampls::AMPLSolverException& e) {
    printf("Caught exception: %s\n\n", e.what());
  }

  createAndSolveInfeasibleModel<T>(false);
  createAndSolveSimpleModel<T>();
}

int main(int argc, char** argv) {

#ifdef USE_cplex
  run<ampls::CPLEXModel>();
#endif
#ifdef USE_xpress
  run<ampls::XPRESSModel>();
#endif
#ifdef USE_gurobi
  run<ampls::GurobiModel>();
#endif
#ifdef USE_cbcmp
  run<ampls::CbcModel>();
#endif
#ifdef USE_copt
  run<ampls::CoptModel>();
#endif
}