// Must be included before ASL, which
// is included by cplex_interface and gurobi_interface
#include "gurobi_c.h"
#include "ilcplex/cplex.h"


#include "cplex_interface.h"
#include "cplex_callback.h"


#include "gurobi_interface.h"
#include "gurobi_callback.h"

#include "simpleapi/simpleApi.h"
#include "test-config.h" // for MODELS_DIR
#include <cstring>
#include <cstdio>

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
    ampls::CBWhere::Where where = getAMPLType();
    switch (where)
    {
    case ampls::CBWhere::msg:
    //  printf("**%s**\n", getMessage());
      return 0;
    case ampls::CBWhere::presolve:
               return 0;
    case ampls::CBWhere::mip:
      break;
    case ampls::CBWhere::mipsol:
    case ampls::CBWhere::mipnode:
    {
      
      // TODO Check why in CPLEX this does not work
      auto sol = getSolutionVector();
      for (int i = 0; i < sol.size(); ++i)
        nnz++;
      //  if(sol[i]!=0)
       printf("Non zeroes: %d\n", nnz);
    }
    nrun++;
      stat=  addLazyIndices(2, ind, val, ampls::CBDirection::ge, 1);
      if (stat)
        printf("ERROR: %s\n", model_->error(stat).c_str());
      return 0;
   // case ampls::CBWhere::notmapped:
    //  printf("Not mapped! Where: %s\n", getWhere());

    }
    return 0;
  }

};

double doStuff2(ampls::AMPLModel& m, const char *name) 
{
  // Set a (generic) callback
  MyGenericCallbackCut cb;
  m.setGenericCallback(&cb);
  
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
void main(int argc, char** argv) {

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "queens18.nl");

  ampls::CPLEXDrv cplex;
  ampls::CPLEXModel cm = cplex.loadModel(buffer);
  doStuff2(cm, "cplex");
  
  ampls::GurobiDrv gurobi;
  ampls::GurobiModel mg = gurobi.loadModel(buffer);
  mg.enableLazyConstraints();
  doStuff2(mg, "gurobi");


  /* TODO: Reassign gurobi model does not work
  //mg.optimize();
  //printf("Objective without callback: %f\n", mg.getObj());
  
  
  mg = gurobi.loadModel(buffer);
  mg.optimize();
  printf("Objective without callback (%s)=%f\n", "gurobi", mg.getObj());
  */
 
}
