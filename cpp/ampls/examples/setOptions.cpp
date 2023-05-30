// Include the AMPL-API
#include "ampl/ampl.h"
// Include AMPLS to interact with solvers
#include "ampls/ampls.h"

/// <summary>
/// Create a non linear AMPL model using the AMPL API
/// </summary>
void makeAmplNQueens(ampl::AMPL &ampl , int size) {
  ampl.eval("param n integer > 0;" // # N-queens
    "var Row{1..n} integer >= 1 <= n;"
    "row_attacks: alldiff({ j in 1..n } Row[j]);"
    "diag_attacks: alldiff({ j in 1..n } Row[j] + j);"
    "rdiag_attacks: alldiff({ j in 1..n } Row[j] - j);");
  ampl.getParameter("n").set(size);

}

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
  const char** loadOptions = nullptr,
  const std::map<std::string, int>& otherOptions = {}) {

    auto model = ampls::AMPLAPIInterface::exportModel<T>(ampl, loadOptions);
    for (auto o : otherOptions)
      model.setOption(o.first.c_str(), o.second);
    model.optimize();
    ampls::AMPLAPIInterface::importModel(ampl, model);
    return model;
}


template <class T> void run(const char* options[]=nullptr) {
  ampl::AMPL a;
  makeNLModel(a);
  try {
    exportAndRun<T>(a, options);
  }
  catch (const std::exception& e) {
    printf("Exception caught=%s\n", e.what());
  }
}

int main(int argc, char** argv) {

#ifdef USE_gurobi
  // Set converter option; must be done before loading the model
  const char* options[] = { "acc:sin=0", nullptr };
  run<ampls::GurobiModel>(options);
  // Try with default options (uses gurobi's SIN function)
  const char* noptions[] = { NULL };
  run<ampls::GurobiModel>(noptions);
#endif
#ifdef USE_cplex
  // Linearization options, no effect after the model is loaded
  const char* ploption[] = { "cvt:plapprox:reltol=0.1", nullptr };
  run<ampls::CPLEXModel>(ploption);
  const char* ploption2[] = { "cvt:plapprox:reltol=0.6", nullptr };
  run<ampls::CPLEXModel>(ploption2);
#endif
#ifdef USE_xpress
  run<ampls::XPRESSModel>();
#endif
#ifdef USE_cbcmp
  run<ampls::CbcModel>();
#endif
#ifdef USE_copt
  run<ampls::CoptModel>();
#endif
}