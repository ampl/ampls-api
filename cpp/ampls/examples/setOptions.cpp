// Include the AMPL-API
#include "ampl/ampl.h"
// Include AMPLS to interact with solvers
#include "ampls/ampls.h"

void makeNLModel(ampl::AMPL& ampl) {
  ampl.eval("param pi := 4 * atan(1);"
    "var x >= -4 * pi, <= -pi;"
    "var y >= -1, <= 1;"
    "s.t.Sin01: y <= sin(x);"
    "maximize SumXY : x + y;");
}
/// <summary>
/// Export the AMPL model to a (templated) AMPLS solver and solve it via AMPLS
/// and reimports the model in the AMPL API object 
/// Note the distinction between loadOptions and otherOptions. The first are 
/// options that are only changeable when the model is exported from AMPL to the 
/// solver object (most notably, all options related to the mp driver reformulations,
/// with names acc:* and cvt:*),
/// the latter are runtime options that can be used in repeated solves.
/// </summary>
template <class T> T exportAndRun(ampl::AMPL& ampl,
  const std::vector<std::string> loadOptions = {},
  const std::map<std::string, int>& otherOptions = {}) {

    auto model = ampls::AMPLAPIInterface::exportModel<T>(ampl, loadOptions);
    for (auto o : otherOptions)
      model.setOption(o.first.c_str(), o.second);
    model.optimize();
    ampls::AMPLAPIInterface::importModel(ampl, model);
    return model;
}


template <class T> void run(const std::vector<std::string> loadOptions = {},
  const std::map<std::string, int>& otherOptions = {}) {
  ampl::AMPL a;
  makeNLModel(a);
  exportAndRun<T>(a, loadOptions, otherOptions);
}
/// <summary>
/// Shows and tests how to set options and how to get their values
/// </summary>
/// <typeparam name="T"></typeparam>
template <class T> void getAndSetOptions() {
  ampl::AMPL a;
  makeNLModel(a);
  auto model = ampls::AMPLAPIInterface::exportModel<T>(a);
  // Options set afterwards
  model.setOption("outlev", 1);
  assert(1 == model.getIntOption("outlev"));
  model.setOption("outlev", 0);
  assert(0 == model.getIntOption("outlev"));

  model.setOption("mip:gap", 0.1);
  assert(0.1 == model.getDoubleOption("mip:gap"));

  model.setOption("tech:logfile", "mylog");
  auto c = model.getStringOption("tech:logfile");
  assert("mylog"==c);

  // Options set at export stage
  auto model2 = ampls::AMPLAPIInterface::exportModel<T>(a, { "cvt:pre:all=1" });
  assert(1 == model2.getIntOption("cvt:pre:all"));
  auto model3 = ampls::AMPLAPIInterface::exportModel<T>(a, { "cvt:pre:all=0" });
  assert(0 == model3.getIntOption("cvt:pre:all"));
}

int main(int argc, char** argv) {
#ifdef USE_gurobi
  getAndSetOptions<ampls::GurobiModel>();
  try {
    run<ampls::GurobiModel>({ "rongname" });
  }
  catch (const std::runtime_error& e) {
    printf("\nAMPLSolverException caught:\n%s\n", e.what());
  }
  // This would have no effect if specified when exporting the model
  // (or when reading the NL file with AMPLModel::load())
  try {
    {
      run<ampls::GurobiModel>({}, { {"cvt:pre:all", 0} });
    }
  }
  catch (const ampls::AMPLSolverException& e) {
    printf("\nAMPLSolverException caught:\n%s\n", e.what());
  }
  // Set converter option; must be when exporting the model
  run<ampls::GurobiModel>({ "acc:sin=0" });

  // Try with default options (uses gurobi's SIN function)
  run<ampls::GurobiModel>({});
#endif
#ifdef USE_cplex
  // This would have no effect if specified when exporting the model
  // (or when reading the NL file with AMPLModel::load())
  try {
    run<ampls::CPLEXModel>({}, { {"cvt:pre:all", 0} });
  }
  catch (const ampls::AMPLSolverException& e) {
    printf("\nAMPLSolverException caught:\n%s\n", e.what());
  }
  // Linearization options, no effect after the model is loaded
  run<ampls::CPLEXModel>({ "cvt:plapprox:reltol=0.1" });
  run<ampls::CPLEXModel>({ "cvt:plapprox:reltol=0.6" });
#endif
#ifdef USE_xpress
  // This would have no effect if specified when exporting the model
  // (or when reading the NL file with AMPLModel::load())
  try {
    run<ampls::XPRESSModel>({}, { {"cvt:pre:all", 0} });
  }
  catch (const ampls::AMPLSolverException& e) {
    printf("\nAMPLSolverException caught:\n%s\n", e.what());
  }
  run<ampls::XPRESSModel>();
#endif
#ifdef USE_cbcmp
  run<ampls::CbcModel>();
#endif
#ifdef USE_copt
  // This would have no effect if specified when exporting the model
// (or when reading the NL file with AMPLModel::load())
  try {
    run<ampls::CoptModel>({}, { {"cvt:pre:all", 0} });
  }
  catch (const ampls::AMPLSolverException& e) {
    printf("\nAMPLSolverException caught:\n%s\n", e.what());
  }
  run<ampls::CoptModel>();
#endif
}