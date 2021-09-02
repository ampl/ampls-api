#ifdef USE_cplex
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_xpress
#include "xpress_interface.h"
#endif

#include <iostream>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"


const char* MODELNAME = "testmodel.nl";

class AMPLAPIInterface
{
public:
#ifdef USE_gurobi
  static ampls::GurobiModel exportGurobiModel(ampl::AMPL& a) {
    a.eval("option auxfiles cr;");
    a.eval("write g___modelexport___;");
    ampls::GurobiDrv gurobi;
    ampls::GurobiModel g = gurobi.loadModel("___modelexport___.nl");
    return g;
  }
#endif

#ifdef USE_cplex
  static ampls::CPLEXModel exportCPLEXModel(ampl::AMPL& a) {
    a.eval("option auxfiles cr;");
    a.eval("write g___modelexport___;");
    ampls::CPLEXDrv gurobi;
    ampls::CPLEXModel g = gurobi.loadModel("___modelexport___.nl");
    return g;
  }

#endif
  static void importModel(ampl::AMPL& a, ampls::AMPLModel& g) {
    g.writeSol();
    a.eval("solution ___modelexport___.sol;");
    a.eval(g.getRecordedConstraints());
    a.eval(g.getRecordedVariables());
  }

};

void printStatistics(ampl::AMPL& ampl) {
  printf("AMPL: I have %d variables and %d constraints\n",
    static_cast<int>(ampl.getValue("_nvars").dbl()),
    static_cast<int>(ampl.getValue("_ncons").dbl()));

}


void printStatistics(ampls::AMPLModel& m, const char* name)
{
  printf("\n%s: I have %d variables and %d constraints\n",
    name, m.getNumVars(), m.getNumCons());
  printf("%s: objective=%f\n", name, m.getObj());
}

void doStuff(ampls::AMPLModel& m, const char* name)
{
  m.optimize();
  printStatistics(m, name);

  // Create a new constraint using the solver interface
  int n = m.getNumVars();
  std::vector<int> indices(n);
  std::vector<double> coeff(n, 1);
  for (int i = 0; i < n; i++)
    indices[i] = i;

  // Add it to the solver and records it for AMPL
  m.recordConstraint(m.addConstraint(n, indices.data(), coeff.data(), ampls::CutDirection::LE, n));
  m.optimize();
  printStatistics(m, name);

  m.recordVariable(m.addVariable(0, NULL, NULL, 0, 10, 100, ampls::VarType::Integer));
  m.optimize();
  printStatistics(m, name);
}


int main(int argc, char** argv) {
  ampl::AMPL ampl;
  ampl.eval("var x >=0; var y>=0;");
  ampl.eval("maximize z: x+y + to_come;");
  ampl.eval("constraint: x+2*y+ to_come<=4;");
  printStatistics(ampl);

#ifdef USE_gurobi
  ampls::GurobiModel g = AMPLAPIInterface::exportGurobiModel(ampl);
  doStuff(g, "gurobi");
#endif
  AMPLAPIInterface::importModel(ampl, g);
  printStatistics(ampl);
#ifdef USE_cplex
  ampls::CPLEXModel c = AMPLAPIInterface::exportCPLEXModel(ampl);
  doStuff(c, "cplex");
#endif

  AMPLAPIInterface::importModel(ampl, g);
  printStatistics(ampl);
  return 0;
 
}
