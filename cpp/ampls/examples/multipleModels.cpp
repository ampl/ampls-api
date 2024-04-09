#include <string>
#include <exception>
#include <cassert>
#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"


/// This example shows how to create AMPL models using the AMPL API and detect various 
/// errors that can arise when importing them to AMPLS

void makeAmplModel(ampl::AMPL &ampl , int numvars, bool unfeasible, bool unbounded=false) {
  assert(!(unfeasible && unbounded));
  ampl.eval("set varIndexes within Integers; set constraintIndexes within Integers;"
    "param varCosts{ varIndexes };"
    "param rightHandSides{ constraintIndexes };"
    "var x{ i in varIndexes } >= 0;"
    "maximize objectiveFunction : sum{ i in varIndexes } varCosts[i] * x[i];");
    

  ampl::DataFrame d(1, { "varIndexes", "varCosts" });
  for (int i = 0; i < numvars; i++)
    d.addRow(i, i);
  ampl.setData(d, "varIndexes");
  if (!unbounded)
  {
    ampl.eval(" subject to mainConstraints{ j in constraintIndexes } : x[j] + x[j + 1] <= rightHandSides[j];");
    ampl::DataFrame d2(1, { "constraintIndexes", "rightHandSides" });
    for (int i = 0; i < numvars - 1; i++)
      d2.addRow(i, unfeasible ? -1 : 1);
    ampl.setData(d2, "constraintIndexes");
  }
}

/// <summary>
/// Export the AMPL model to a (templated) AMPLS solver and solve it via AMPLS
/// Reimports the model in the AMPL API object and also returns the AMPLS object for
/// further analysis 
/// </summary>
template <class T> T solveModel(ampl::AMPL& ampl, const std::map<std::string, int>& options = {}) {
    auto model = ampls::AMPLAPIInterface::exportModel<T>(ampl);
    for (auto o : options)
    {
      try {
        model.setOption(o.first.c_str(), o.second);
      }
      catch (...) {}
    }
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
  assert(status == CPX_STAT_OPTIMAL);
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

  // Get status through AMPLS
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

template <class T> void createAndSolveTrivialModel() {
  ampl::AMPL ampl;
  ampl.eval("var x; maximize z:x; c:x=5;");
  auto model = solveModel<T>(ampl);
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

template <class T> void createAndSolveUnboundedModel(bool presolve) {
  int numVars = 10;
  ampl::AMPL ampl;
  makeAmplModel(ampl, numVars, false, true);
  ampl.setIntOption("presolve", presolve ? 100 : 0); // 10 is the default number of presolve passes
  std::map<std::string, int> options = { {"outlev", 1}};
  auto model = solveModel<T>(ampl, options);
  assert(model.getStatus() == ampls::Status::UNBOUNDED);
  printf("Completed Unbounded Model Test\n\n");
}

template <class T> void example() {

  createAndSolveInfeasibleModel<T>(false);

  // Normal model solver to completion
  createAndSolveSimpleModel<T>();
  bool caught = false;
  try {
    // This will fail because AMPL's presolve will block the 
    // export of the model
    createAndSolveInfeasibleModel<T>(true);
  }
  catch (const ampl::InfeasibilityException& e) {
    printf("Completed Infeasible presolved Model Test\n\n");
    printf("Caught exception:\n%s\n", e.what());
    caught = true;
  }
  assert(caught);
  caught = false;
  try {
    createAndSolveTrivialModel<T>();
  }
  catch (const ampl::PresolveException& e) {
    printf("Completed Trivial Model Test\n");
    printf("Caught exception:\n%s\n", e.what());
    caught = true;
  }
  assert(caught);

  

  // Unbounded models are exported either way
  createAndSolveUnboundedModel<T>(true);
  createAndSolveUnboundedModel<T>(false);


}

int main(int argc, char** argv) {
#ifdef USE_highs
  example<ampls::HighsModel>();
#endif
#ifdef USE_cplex
  example<ampls::CPLEXModel>();
#endif
#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif
#ifdef USE_gurobi
  example<ampls::GurobiModel>();
#endif
#ifdef USE_cbcmp
  example<ampls::CbcModel>();
#endif

#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif
#ifdef USE_copt
  //example<ampls::CoptModel>();
#endif
  return 0;
}