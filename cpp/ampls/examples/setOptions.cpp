// Include the AMPL-API
#include "ampl/ampl.h"
// Include AMPLS to interact with solvers
#include "ampls/ampls.h"
class My : public ampl::OutputHandler {
  public:
    virtual void output(ampl::output::Kind kind, const char* msg) {
      printf("%s\n", msg);
    }
};

class MyEH : public ampl::ErrorHandler {
  virtual void error(const ampl::AMPLException& e) {
    printf("%s\n", e.getMessage());
  }
  virtual void warning(const ampl::AMPLException& e) {
    printf("%s\n", e.getMessage());

    }

};
void makeNLModel(ampl::AMPL& ampl) {
  My oh;
  MyEH eh;
  ampl.setOutputHandler(&oh);
  ampl.setErrorHandler(&eh);
  ampl.cd();
  ampl.eval("cd;");
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
  model.setOption("mip:return_gap", 7);
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
template<class T> void example() {

  getAndSetOptions<T>();

  // This would have effect only if specified when exporting the model
  // (or when reading the NL file with AMPLModel::load())
  try {
    run<T>({}, { {"cvt:pre:all", 0} });
  }
  catch (const ampls::AMPLSolverException& e) {
    printf("\nAMPLSolverException caught:\n%s\n", e.what());
  }
  // This will throw an exepction
  try {
    run<T>({ "wrongname" });
  }
  catch (const std::runtime_error& e) {
    printf("\nAMPLSolverException caught:\n%s\n", e.what());
  }
#ifdef USE_cplex
  run<ampls::CPLEXModel>({ "cvt:plapprox:reltol=0.1" });
  run<ampls::CPLEXModel>({ "cvt:plapprox:reltol=0.6" });
#endif
}

int main(int argc, char** argv) {
#ifdef USE_gurobi
  example<ampls::GurobiModel>();
#endif

#ifdef USE_highs
  example<ampls::HighsModel>();
#endif

#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif

#ifdef USE_cplex
//  example< ampls::CPLEXModel>();
#endif

#ifdef USE_copt
  example<ampls::CoptModel>();
#endif

#ifdef USE_cbcmp
  example<ampls::CbcModel>();
#endif

#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif

  return 0;
}
