#ifdef USE_cplex
//#include "cplex_interface.h"
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


double doStuff(ampls::AMPLModel& m, const char *name) 
{
  // Create a new constraint using the solver interface
  int n = m.getNumVars();
  std::vector<int> indices(n);
  std::vector<double> coeff(n);
  for (int i = 0; i < n; i++)
  {
    indices[i] = i;
    coeff[i] =1;
  }
  // Add it to the solver and records it for AMPL
  m.recordConstraint("c1", indices, coeff,
    ampls::CutDirection::LE, 1);
  
  m.recordVariable("v1", indices, coeff, 0, 10, 100, ampls::VarType::Integer);

  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", name, obj);
  return obj;
}

class AMPLAPIInterface
{
public:
  static ampls::GurobiModel exportGurobiModel(ampl::AMPL& a) {
    a.eval("option auxfiles cr;");
    a.eval("write g___modelexport___;");
    ampls::GurobiDrv gurobi;
    ampls::GurobiModel g = gurobi.loadModel("___modelexport___.nl");
    return g;
  }

  static void importGurobiModel(ampl::AMPL& a, ampls::GurobiModel& g) {
    g.writeSol();
    a.eval("solution ___modelexport___.sol;");
    a.eval(g.getRecordedConstraints());
    a.eval(g.getRecordedVariables());
  }
};



int main(int argc, char** argv) {
  ampl::AMPL ampl;
  ampl.eval("var x >=0; var y>=0;");
  ampl.eval("maximize z: x+y + to_come;");
  ampl.eval("constraint: x-2*y+ to_come<=4;");
  
  printf("AMPL: I have %d variables and %d constraints\n",
    static_cast<int>(ampl.getValue("_nvars").dbl()),
    static_cast<int>(ampl.getValue("_ncons").dbl()));
  ampls::GurobiModel g = AMPLAPIInterface::exportGurobiModel(ampl);
  printf("Solver: I have %d variables and %d constraints\n", 
    g.getNumVars(), g.getIntAttr(GRB_INT_ATTR_NUMCONSTRS));
  
  // Use it as generic model
  doStuff(g, "gurobi");

  printf("Solver: I have %d variables and %d constraints\n",
    g.getNumVars(), g.getIntAttr(GRB_INT_ATTR_NUMCONSTRS));
  AMPLAPIInterface::importGurobiModel(ampl, g);
  printf("AMPL: I have %d variables and %d constraints\n",
    static_cast<int>(ampl.getValue("_nvars").dbl()),
    static_cast<int>(ampl.getValue("_ncons").dbl()));

  return 0;
 
}
