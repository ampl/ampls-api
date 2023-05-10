#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include<iostream>
#include<string>
#include <cstring>
const char* MODELNAME = "tsp.nl";

class CCB : public ampls::GenericCallback
{
  virtual int run()
  {
    // printf("Called from %s\n", getWhere(whereFrom));
    switch (getAMPLSWhere())
    {
    case ampls::Where::MSG:
  //    printf("**%s**\n", getMessage());
      return 0;
    case ampls::Where::MIPSOL:
    case ampls::Where::MIPNODE:
      try {
        printf("MIPSOL OBJ = %f\n", getObj());
      }
      catch (const ampls::AMPLSolverException& s)
      {
        printf("%s", s.what());
      }
    }
    return 0;
  }

};

void doStuff(ampls::AMPLModel& m, const char *name)
{
  int nOptions = 3;
  /*for (auto o : m.getOptions())
  {
    std::cout << o.toString() << std::endl;
    if (nOptions-- == 0)
      break;
  }*/
  CCB b;
  // Set the generic function above as callback
  m.setCallback(&b);
  // Optimize
  m.optimize();
  // Get ojbjective
  double obj = m.getObj();
  printf("Solution with optimizer %s=%f\n", name, obj);
}




int main(int argc, char** argv) {
  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);


#ifdef USE_copt
  // Load a model using copt
  ampls::CoptDrv copt;
  ampls::CoptModel cm = copt.loadModel(buffer);
  // Use it as generic model
  doStuff(cm, "copt");
#endif


#ifdef USE_gurobi
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel g = gurobi.loadModel(buffer);
  // Use it as generic model
  doStuff(g, "gurobi");
#endif



#ifdef USE_cplexmp
  // Load a model using CPLEX driver
  ampls::CPLEXDrv cplex;
  ampls::CPLEXModel c = cplex.loadModel(buffer);
  // Use it as generic model
  doStuff(c, "cplex");
#endif



#ifdef USE_xpress
  // Load a model using CPLEX driver
  ampls::XPRESSDrv xpress;
  ampls::XPRESSModel x = xpress.loadModel(buffer);
  // Use it as generic model
  doStuff(x, "xpress");
#endif
  

#ifdef USE_cbcmp
  // Load a model using gurobi driver
  ampls::CbcDrv cbc;
  ampls::CbcModel cbcmodel = cbc.loadModel(buffer);
  // Use it as generic model
  doStuff(cbcmodel, "cbc");
#endif

}
