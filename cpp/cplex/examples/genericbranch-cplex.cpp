#include "cplex_interface.h"

#include "test-config.h" // for MODELS_DIR

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <mutex>

// This is a C example taken from IBM ILOG CPLEX Optimization Studio examples
// Original name: genericbranch.c
// It is provided as is - with the only addition of a mutex to guard CALLBACKDATA::calls
// and CALLBACKDATA::branches - to show how to use the original CPLEX C API with AMPLS.

/* Data that is used within the callback. */
typedef struct {
  char* ctype;    /* Variable types. */
  int cols;       /* Number of variables. */
  int nthreads;   /* Maximum number of threads we can handle. */
  double** relx;  /* Per-thread buffer for reading variable values. */
  int calls;      /* How often was the callback invoked? */
  int branches;   /* How many branches did we create */

  std::mutex mutex;  // Mutex for synchronization
} CALLBACKDATA;

void increment_calls(CALLBACKDATA* cbdata) {
  std::lock_guard<std::mutex> lock(cbdata->mutex);  // Acquire lock
  cbdata->calls++;
}  // Release lock automatically when lock_guard goes out of scope

void increment_branches(CALLBACKDATA* cbdata) {
  std::lock_guard<std::mutex> lock(cbdata->mutex);  // Acquire lock
  cbdata->branches++;
}  // Release lock automatically when lock_guard goes out of scope

/* Generic callback that implements most infeasible branching. */
static int CPXPUBLIC
branchcallback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
  void* cbhandle)
{
  CALLBACKDATA* data = (CALLBACKDATA*)cbhandle;
  CPXINT threadid;
  CPXLONG depth;
  int status, lpstat;
  double obj;

  (void)contextid; /* unused */

  /* NOTE: Strictly speaking, the increment of data->calls and data->branches
   *       should be protected by a lock/mutex/semaphore. However, to keep
   *       the code simple we don't do that here.
   */
  increment_calls(data);

  /* Get the id of the thread on which the callback is invoked.
   * We need this to pick the right buffer in data->relx.
   */
  status = CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID,
    &threadid);
  if (status) {
    fprintf(stderr, "Failed to get thread id: %d\n", status);
    return status;
  }

  /* For sake of illustration prune every node that has a depth larger
   * than 1000.
   */
  status = CPXcallbackgetinfolong(context, CPXCALLBACKINFO_NODEDEPTH,
    &depth);
  if (status) {
    fprintf(stderr, "Failed to get node depth: %d\n", status);
    return status;
  }
  if (depth > 1000) {
    status = CPXcallbackprunenode(context);
    if (status) {
      fprintf(stderr, "Failed to prune node: %d\n", status);
      return status;
    }

    return 0;
  }

  /* Get the solution status of the current relaxation.
   */
  status = CPXcallbackgetrelaxationstatus(context, &lpstat, 0);
  if (status) {
    fprintf(stderr, "Failed to get relaxation status: %d\n", status);
    return status;
  }

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

    status = CPXcallbackgetrelaxationpoint(context, data->relx[threadid],
      0, data->cols - 1, &obj);
    if (status) {
      fprintf(stderr, "Failed to get relaxation: %d\n", status);
      return status;
    }

    for (j = 0; j < data->cols; ++j) {
      if (data->ctype[j] != CPX_CONTINUOUS &&
        data->ctype[j] != CPX_SEMICONT)
      {
        double intval = round(data->relx[threadid][j]);
        double frac = fabs(intval - data->relx[threadid][j]);

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
      double const up = ceil(data->relx[threadid][maxvar]);
      double const down = floor(data->relx[threadid][maxvar]);

      /* Create the UP branch. */
      status = CPXcallbackmakebranch(context, 1, &maxvar, "L", &up,
        0, 0, NULL, NULL, NULL, NULL, NULL,
        obj, &upchild);
      if (status) {
        fprintf(stderr, "Failed to create up branch: %d\n", status);
        return status;
      }
      increment_branches(data);

      /* Create the DOWN branch. */
      status = CPXcallbackmakebranch(context, 1, &maxvar, "U", &down,
        0, 0, NULL, NULL, NULL, NULL, NULL,
        obj, &downchild);
      if (status) {
        fprintf(stderr, "Failed to create down branch: %d\n", status);
        return status;
      }
      increment_branches(data);

      printf("Thread %d: created Branches: UP=%d, DOWN=%d\n", threadid, upchild, downchild);
      
      /* We don't use the unique ids of the down and up child. We only
       * have them so illustrate how they are returned from
       * CPXcallbackmakebranch().
       */
      (void)downchild;
      (void)upchild;
    }
  }

  return 0;
} /* END branchcallback */



int
main(int argc, char* argv[])
{
  /* This section replaces the creation the model directly in CPLEX */
  std::string md(MODELS_DIR);
  md += "10teams.nl";
  const char* options[2] = { "outlev=1", nullptr };
  auto cplexmodel = ampls::AMPLModel::load<ampls::CPLEXModel>(md.c_str(), options);
  /* The outcome of that are two pointers to the native CPLEX environment
     and lp structs */
  auto env = cplexmodel.getCPXENV();
  auto lp = cplexmodel.getCPXLP();

  
  /* From here onwards, the CPLEX example is untouched */

  int status, t;
  double objval;
  CALLBACKDATA data = { NULL, 0, 0, NULL, 0, 0 };

  status = CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON);
  if (status) {
    fprintf(stderr, "Failed to turn enable screen output: %d\n", status);
    goto TERMINATE;
  }

  /* Read the column types into the data that we will pass into the callback. */
  data.cols = CPXgetnumcols(env, lp);
  data.ctype = (char*)malloc(data.cols * sizeof(*data.ctype));
  if (data.ctype == NULL) {
    status = CPXERR_NO_MEMORY;
    fprintf(stderr, "Out of memory\n");
    goto TERMINATE;
  }
  status = CPXgetctype(env, lp, data.ctype, 0, data.cols - 1);
  if (status) {
    fprintf(stderr, "Failed to query variable types: %d\n", status);
    goto TERMINATE;
  }

  /* For each potential thread that CPLEX may use, create a buffer into
   * which the callback can read the current relaxation.
   * We don't change the CPXPARAM_Threads parameter, so CPLEX will use
   * up to as many threads as there are cores.
   */
  status = CPXgetnumcores(env, &data.nthreads);
  if (status) {
    fprintf(stderr, "Failed to get number of cores: %d\n", status);
    goto TERMINATE;
  }
  data.relx = (double**)calloc(data.nthreads, sizeof(*data.relx));
  if (data.relx == NULL) {
    status = CPXERR_NO_MEMORY;
    fprintf(stderr, "Out of memory\n");
    goto TERMINATE;
  }
  for (t = 0; t < data.nthreads; ++t) {
    data.relx[t] = (double*)malloc(data.cols * sizeof(*data.relx[t]));
    if (data.relx[t] == NULL) {
      status = CPXERR_NO_MEMORY;
      goto TERMINATE;
    }
  }

  /* Register the callback function. */
  status = CPXcallbacksetfunc(env, lp, CPX_CALLBACKCONTEXT_BRANCHING,
    branchcallback, &data);
  if (status) {
    fprintf(stderr, "Failed to set callback: %d\n", status);
    goto TERMINATE;
  }

  /* Limit the number of nodes.
   * The branching strategy implemented here is not smart so solving even
   * a simple MIP may turn out to take a long time.
   */
  status = CPXsetintparam(env, CPXPARAM_MIP_Limits_Nodes, 1000);
  if (status) {
    fprintf(stderr, "Failed to set node limit: %d\n", status);
    goto TERMINATE;
  }

  /* Solve the model. */
  status = CPXmipopt(env, lp);
  if (status) {
    fprintf(stderr, "Optimization failed: %d\n", status);
    goto TERMINATE;
  }

  /* Report some statistics. */
  printf("Model solved, solution status = %d\n", CPXgetstat(env, lp));
  status = CPXgetobjval(env, lp, &objval);
  if (status)
    printf("No objective value (error = %d\n", status);
  else
    printf("Objective = %f\n", objval);
  printf("Callback was invoked %d times and created %d branches\n",
    data.calls, data.branches);

TERMINATE:
  /* Cleanup */
  if (data.relx) {
    for (t = 0; t < data.nthreads; ++t)
      free(data.relx[t]);
    free(data.relx);
  }
  free(data.ctype);
  CPXfreeprob(env, &lp);
  CPXcloseCPLEX(&env);
  if (status)
    return status;
  return 0;
} /* END main */
