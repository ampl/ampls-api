#ifdef USE_cplex
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_xpress
#include "xpress_interface.h"
#endif

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 
/*
set A;
var scalar >= 0, <= 4;
var x{a in A} >=0;
var y{a in A, b in A}>=0;
maximize z: scalar + sum{a in A} x[a] + sum{a in A, b in A} y[a,b]*0.1;
c{a in A}: x[a] + sum{b in A} y[a,b] <= 42;

data;

set A := 1 2 3 aa bb cc 'a' 'b' 'c' "d" "e" "f" "4" '5' 6 'a a' "a b" 'ab[c]' "de[f]" 'ab"c' "ab'c";
*/
class MyGenericCallbackCut : public ampls::GenericCallback
{
  int nrun;
public:
  MyGenericCallbackCut() : nrun(0) {}
  virtual int run()
  {
    int ind[] = { 1 + nrun,36+nrun};
    int stat = 0;
    int nnz = 0;
    double val[] = { 1,1 };
    // Get the generic mapping
    ampls::Where::CBWhere where = getAMPLWhere();
    switch (where)
    {
    case ampls::Where::MSG:
      printf("**%s**\n", getMessage());
      return 0;
    case ampls::Where::PRESOLVE:
               return 0;
    case ampls::Where::MIP:
      break;
    case ampls::Where::MIPSOL:
    case ampls::Where::MIPNODE:
    {
      // TODO Check why in CPLEX this does not work
      auto sol = getSolutionVector();
      for (int i = 0; i < sol.size(); ++i)
        if(sol[i]!=0) nnz++;
      printf("Non zeroes: %d\n", nnz);
    }
    nrun++;
      stat=  addLazyIndices(2, ind, val, ampls::CutDirection::GE, 1);
      if (stat)
        printf("ERROR: %s\n", model_->error(stat).c_str());
      return 0;
   // case ampls::CBWhere::notmapped:
    //  printf("Not mapped! Where: %s\n", getWhere());

    }
    return 0;
  }

};

double doStuff(ampls::AMPLModel& m, const char *name) 
{
  // Set a (generic) callback
  MyGenericCallbackCut cb;
  m.setCallback(&cb);
  
  // Start the optimization process
  m.optimize();
  // Get the objective value
  double obj = m.getObj();
  printf("Objective with callback (%s)=%f\n", name, obj);
  // Get the solution vector
  std::size_t nr = m.getNumVars();
  std::vector<double> solution(nr);
  m.getSolution(0, nr, solution.data());
  int nnz = 0;
  //for (int i = 0; i < nr; i++)
    //if (solution[i] != 0) nnz++;
  //m.printModelVars(true);
//  printf("\nNumber of non zeroes = %d\n", nnz);

  // Write the AMPL sol file
  m.writeSol();
  return obj;
}
int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "queens18.nl");

#ifdef USE_gurobi
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel g = gurobi.loadModel(buffer);
  g.enableLazyConstraints();
  // Use it as generic model
  doStuff(g, "gurobi");
#endif

#ifdef USE_cplex
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

  /* TODO: Reassign gurobi model does not work
  //mg.optimize();
  //printf("Objective without callback: %f\n", mg.getObj());
  
  
  mg = gurobi.loadModel(buffer);
  mg.optimize();
  printf("Objective without callback (%s)=%f\n", "gurobi", mg.getObj());
  */
 
}
