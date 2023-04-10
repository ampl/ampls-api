#include "copt_interface.h"
#include "ampls/ampls.h"

#include "copt.h"

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 
#include <cmath>

const char* MODELNAME = "tsp.nl";

class MyCoptCallback : public ampls::CoptCallback
{
  public:
  int _lastIter;
  int _numVars;
  MyCoptCallback(int numVars) : _lastIter(0), _numVars(numVars) {
  }
  int count(const std::vector<double> sol) {
    int nnz = 0;
    for (auto d : sol)
      if (d != 0) nnz++;
    return nnz;
  }

  virtual int run()
  {
    // Get the data used in a copt C native callback
    int where = getWhere();
    void* cbdata = getCBData();



    if (where == ampls::Where::MSG)
    {
      // Copt has a different callback for messages,
      // so we use the ampls generic functions here
      std::string s = getMessage();
      printf("%s", s.data());
      return 0;
    }
    double v;
    printf("In %s\n", getWhereString());
    if (where == COPT_CBCONTEXT_MIPSOL) {
      auto info = { COPT_CBINFO_BESTOBJ, COPT_CBINFO_HASINCUMBENT, COPT_CBINFO_MIPCANDOBJ };
      for (const char* i : info) {
        COPT_GetCallbackInfo(cbdata, i, &v);
        printf("%s=%f\n", i, v);
      }
      auto sol = std::vector<double>(_numVars);
      COPT_GetCallbackInfo(cbdata, COPT_CBINFO_INCUMBENT, sol.data());
      printf("Incumbent has %d non-zeroes\n", count(sol));
      COPT_GetCallbackInfo(cbdata, COPT_CBINFO_MIPCANDIDATE, sol.data());
      printf("MIP-candidate has %d non-zeroes\n", count(sol));
    }
    else if(where == COPT_CBCONTEXT_MIPRELAX) {
        COPT_GetCallbackInfo(cbdata, COPT_CBINFO_RELAXSOLOBJ, &v);
        printf("COPT_CBINFO_RELAXSOLOBJ=%f\n",v);
      auto sol = std::vector<double>(_numVars);
      COPT_GetCallbackInfo(cbdata, COPT_CBINFO_RELAXSOLUTION, sol.data());
      printf("Incumbent has %d non-zeroes\n", count(sol));
    }

    // Use ampls shortcuts
      double objBest = getDouble(COPT_CBINFO_BESTOBJ);
      double objBnd = getDouble(COPT_CBINFO_BESTBND);
      if (fabs(objBest - objBnd) < 0.1 * (1.0 + fabs(objBest))) {
        printf("Stop early - 10%% gap achieved\n");
        return 1;
      }
    printf("** End of callback handler **\n\n");
    return 0;
    }
};


int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  // Load a model using copt driver
  ampls::CoptDrv copt;
  ampls::CoptModel model = copt.loadModel(buffer);
  
  // Create copt callback and set it using ampls interface
  MyCoptCallback cb(model.getNumVars());
  model.setCallback(&cb);
  
  copt_prob* prob=  model.getCOPTmodel();

  // Start optimization copt's functions 
  COPT_Solve(prob);
  
  // Access objective function through generic API
  double obj = model.getObj();
  printf("Objective: %f\n", obj);
  return 0;
}
