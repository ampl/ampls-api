#include "highs_interface.h"
#include "ampls/ampls.h"

#include "highs.h"

#include "test-config.h" // for MODELS_DIR

#include <cstring> // for strcat 
#include <cmath>

const char* MODELNAME = "tsp.nl";

class MyHighsCallback : public ampls::HighsCallback
{
  public:
  int _lastIter;
  int _numVars;
  MyHighsCallback(int numVars) : _lastIter(0), _numVars(numVars) {
  }
  int count(const std::vector<double> sol) {
    int nnz = 0;
    for (auto d : sol)
      if (d != 0) nnz++;
    return nnz;
  }

  virtual int run()
  {
    // Get the data used in a highs C native callback
    int where = getWhere();
    void* cbdata = getCBData();



    if (where == ampls::Where::MSG)
    {
      // Highs has a different callback for messages,
      // so we use the ampls generic functions here
      std::string s = getMessage();
      printf("%s", s.data());
      return 0;
    }
    double v;
    printf("In %s\n", getWhereString());
    if (where == HIGHS_CBCONTEXT_MIPSOL) {
      auto info = { HIGHS_CBINFO_BESTOBJ, HIGHS_CBINFO_HASINCUMBENT, HIGHS_CBINFO_MIPCANDOBJ };
      for (const char* i : info) {
        HIGHS_GetCallbackInfo(cbdata, i, &v);
        printf("%s=%f\n", i, v);
      }
      auto sol = std::vector<double>(_numVars);
      HIGHS_GetCallbackInfo(cbdata, HIGHS_CBINFO_INCUMBENT, sol.data());
      printf("Incumbent has %d non-zeroes\n", count(sol));
      HIGHS_GetCallbackInfo(cbdata, HIGHS_CBINFO_MIPCANDIDATE, sol.data());
      printf("MIP-candidate has %d non-zeroes\n", count(sol));
    }
    else if(where == HIGHS_CBCONTEXT_MIPRELAX) {
        HIGHS_GetCallbackInfo(cbdata, HIGHS_CBINFO_RELAXSOLOBJ, &v);
        printf("HIGHS_CBINFO_RELAXSOLOBJ=%f\n",v);
      auto sol = std::vector<double>(_numVars);
      HIGHS_GetCallbackInfo(cbdata, HIGHS_CBINFO_RELAXSOLUTION, sol.data());
      printf("Incumbent has %d non-zeroes\n", count(sol));
    }

    // Use ampls shortcuts
      double objBest = getDouble(HIGHS_CBINFO_BESTOBJ);
      double objBnd = getDouble(HIGHS_CBINFO_BESTBND);
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

  // Load a model using highs driver
  ampls::HighsDrv highs;
  ampls::HighsModel model = highs.loadModel(buffer);
  
  // Create highs callback and set it using ampls interface
  MyHighsCallback cb(model.getNumVars());
  model.setCallback(&cb);
  
  highs_prob* prob=  model.getHighsModel();

  // Start optimization highs's functions 
  HIGHS_Solve(prob);
  
  // Access objective function through generic API
  double obj = model.getObj();
  printf("Objective: %f\n", obj);
  return 0;
}
