#include "cplex_interface.h"

#include "test-config.h" // for MODELS_DIR

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <mutex>

// This is a C example taken from IBM ILOG CPLEX Optimization Studio examples
// Original name: genericbranch.c
// This version reorganizes the callback in a more idiomatic way, thus deriving from
// ampls::CPLEXCallback and have the logic of the branch and bound in its run function.
// IMPORTANT This example shows the use of the thread-aware functions in the CPLEXCallback
// class.

class BranchCallback : public ampls::CPLEXCallback {

  /* Data that is used within the callback. */
  struct CALLBACKDATA {
    char* ctype;    /* Variable types. */
    int cols;       /* Number of variables. */
    int nthreads;   /* Maximum number of threads we can handle. */
    double** relx;  /* Per-thread buffer for reading variable values. */
    int calls=0;      /* How often was the callback invoked? */
    int branches=0;   /* How many branches did we create */

    std::mutex mutex;  // Mutex for synchronization
  };
  CALLBACKDATA data_;

public:
  
  void init_callback_data(CPXENVptr env, CPXLPptr lp, int nthreads) {
    /* Read the column types into the data that we will pass into the callback. */
    data_.cols = model_->getNumVars();
    data_.ctype = (char*)malloc(data_.cols * sizeof(*data_.ctype));
    CPXgetctype(env, lp, data_.ctype, 0, data_.cols - 1);
    data_.nthreads = nthreads;
    data_.relx = (double**)calloc(data_.nthreads, sizeof(*data_.relx));

    for (int t = 0; t < data_.nthreads; ++t) {
      data_.relx[t] = (double*)malloc(data_.cols * sizeof(*data_.relx[t]));
      data_.relx[t][0] = t;
    }
  }
  void destroy_callback_data() {
    if (data_.relx) {
      for (int t = 0; t < data_.nthreads; ++t)
        free(data_.relx[t]);
      free(data_.relx);
    }
    free(data_.ctype);
  }
  ~BranchCallback() { destroy_callback_data(); }

 

  // Thread-safe increment the specified field
  void increment(int& field) {
    std::lock_guard<std::mutex> lock(data_.mutex);
    field++;
  }

  int run(int threadid) {
    int status, lpstat;
    double obj;
    
    if (this->getWhere(threadid) != CPX_CALLBACKCONTEXT_BRANCHING)
      return 0;
    increment(data_.calls);

    /* Get the id of the thread on which the callback is invoked.
     * We need this to pick the right buffer in data->relx and in the
     * callback thread-aware functions
     */
    CPXLONG depth;
    auto context = getCPXContext(threadid);

    /* For sake of illustration prune every node that has a depth larger
     * than 1000.
     */

     //status = CPXcallbackgetinfolong(getCPXContext(), CPXCALLBACKINFO_NODEDEPTH,
     //      &depth);
    depth = getCPLEXLong(CPXCALLBACKINFO_NODEDEPTH, threadid);

    if (depth > 1000) {
      CPXcallbackprunenode(context);
    }

    /* Get the solution status of the current relaxation.
     */
    CPXcallbackgetrelaxationstatus(context, &lpstat, 0);

    /* Only branch if the current node relaxation could be solved to optimality.
     * If there was any sort of trouble then don't do anything and thus let
     * CPLEX decide how to cope with that.
     */
    if (lpstat == CPX_STAT_OPTIMAL ||
      lpstat == CPX_STAT_OPTIMAL_INFEAS) {
      /* Relaxation is optimal. Find the integer variable that is most
       * fractional.
       */
      int j;
      double maxfrac = 0.0;
      int maxvar = -1;

      CPXcallbackgetrelaxationpoint(context, data_.relx[threadid],
        0, data_.cols - 1, &obj);

      for (j = 0; j < data_.cols; ++j) {
        if (data_.ctype[j] != CPX_CONTINUOUS &&
          data_.ctype[j] != CPX_SEMICONT)
        {
          double intval = round(data_.relx[threadid][j]);
          double frac = fabs(intval - data_.relx[threadid][j]);

          if (frac > maxfrac) {
            maxfrac = frac;
            maxvar = j;
          }
        }
      }

      /* If the maximum fractionality of all integer variables is small then
       * don't create a custom branch. Instead let CPLEX decide how to branch.
       */
      if (maxfrac > 0.1) {
        /* There is a variable with a sufficiently fractional value.
         * Branch on that variable.
         */
        int downchild, upchild;
        double const up = ceil(data_.relx[threadid][maxvar]);
        double const down = floor(data_.relx[threadid][maxvar]);

        /* Create the UP branch. */
        status = CPXcallbackmakebranch(context, 1, &maxvar, "L", &up,
          0, 0, NULL, NULL, NULL, NULL, NULL,
          obj, &upchild);
        if (status) {
          fprintf(stderr, "Failed to create up branch: %d\n", status);
          return status;
        }
        increment(data_.branches);

        /* Create the DOWN branch. */
        status = CPXcallbackmakebranch(context, 1, &maxvar, "U", &down,
          0, 0, NULL, NULL, NULL, NULL, NULL,
          obj, &downchild);
        if (status) {
          fprintf(stderr, "Failed to create down branch: %d\n", status);
          return status;
        }
        increment(data_.branches);

        printf("Thread %d: created Branches: UP=%d, DOWN=%d\n", threadid, upchild, downchild);
      }
    }
  }
    
  // Single threaded invocation uses this function
  // It is present for compatibility with the GenericCallback interface
  int run() { 
    
    return run(0);
  }

  int calls() { return data_.calls;  }

  int branches() { return data_.branches; }
};

int main(int argc, char* argv[])
{
  /* This section replaces the creation the model directly in CPLEX */
  std::string md(MODELS_DIR);
  md += "10teams.nl";
  const char* options[2] = { "outlev=1", nullptr };
  auto cplexmodel = ampls::AMPLModel::load<ampls::CPLEXModel>(md.c_str(), options);
  
  
  BranchCallback cb;
  /* Register the callback function. */
  cplexmodel.setCallback(&cb);

  // Set the number of threads here. Set it to 1 to have single-threaded
  // execution
  cplexmodel.setOption("threads", 8);

  // Set data depending on the value above
  // This returns either the number of cores of the value of the 
  // option threads, if specified
  int nthreads = cb.getMaxThreads();
  if(nthreads>1) cb.enableThreadsSupport(nthreads);

  /* Pass native structs to initialize data directly from the CPLEX object,
     reusing the logic of the original example */
  auto env = cplexmodel.getCPXENV();
  auto lp = cplexmodel.getCPXLP();
  cb.init_callback_data(env, lp, nthreads);

  /* Limit the number of nodes.
   * The branching strategy implemented here is not smart so solving even
   * a simple MIP may turn out to take a long time.
   */
  cplexmodel.setParam(CPXPARAM_MIP_Limits_Nodes, 1000);
  cplexmodel.setParam(CPXPARAM_ScreenOutput, CPX_ON);

  int status;
  double objval;

  /* Solve the model. */
  status = CPXmipopt(env, lp);

  /* Report some statistics. */
  printf("Model solved, solution status = %d\n", CPXgetstat(env, lp));
  
  status = CPXgetobjval(env, lp, &objval);
  if (status)
    printf("No objective value (error = %d\n", status);
  else
    printf("Objective = %f\n", objval);

  printf("Callback was invoked %d times and created %d branches\n",
    cb.calls(), cb.branches());

} 
