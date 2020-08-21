#ifndef CPLEX_CALLBACK_H_INCLUDE_
#define CPLEX_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "simpleapi/simpleApi.h"

#include "ilcplex/cplex.h"

namespace ampl
{
class CPLEXModel;

class CPLEXCallback : public BaseCallback {
  char CODE[60];

  friend class CPLEXModel;
  friend void msg_callback_wrapper(void* handle, const char* msg);
  friend int lp_callback_wrapper(CPXCENVptr env,
    void* lp, int wf, void* cbh);
  friend CPLEXCallback* setDefaultCB(CPXCENVptr env, void* cbdata,
    int wherefrom, void* userhandle);
  friend int cut_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
    void* cbhandle, int* useraction_p);
  friend int incumbent_callback_wrapper(CPXCENVptr env,
    void* cbdata, int wherefrom, void* cbhandle,
    double objval, double* x, int* isfeas_p,
    int* useraction_p);

  // Callback data
  int wherefrom_;
  CPXCENVptr env_;
  // Stores the pointer to the CPLEX model being used, as passed from the callback
  void* lp_;
  // Stores the pointer to the data passed from CPLEX to the callback
  void* cbdata_;
  // Stores the message passed from CPLEX msg function to the callback
  const char* msg_;

  // Stores obj and solution values when incumbent found
  double objval_;
  double* x_;

protected:
  // Interface
  int doAddCut(int nvars, const int* vars,
    const double* coeffs, char direction, double rhs,
    int type);
  CPXCENVptr env() { return env_; }
  void* cbdata() { return cbdata_; }
public:

  virtual int run(int whereFrom) = 0;
  ~CPLEXCallback() {};

  // Interface
  using BaseCallback::getSolutionVector;
  int getSolution(int len, double* sol);
  double getObjective();
  const char* getWhere(int wherefrom);
  const char* getMessage();

  AMPLCBWhere::Where getAMPLType() {

    switch (wherefrom_)
    {
    case -1:
      return  AMPLCBWhere::msg;
    case CPX_CALLBACK_PRESOLVE:
      return AMPLCBWhere::presolve;
    case CPX_CALLBACK_PRIMAL:
    case CPX_CALLBACK_DUAL:
    case CPX_CALLBACK_BARRIER:
      return AMPLCBWhere::lpsolve;
    case CPX_CALLBACK_MIP_NODE:
      return AMPLCBWhere::mipnode;
    case CPX_CALLBACK_MIP_INCUMBENT_NODESOLN:
    case CPX_CALLBACK_MIP_INCUMBENT_HEURSOLN:
    case CPX_CALLBACK_MIP_INCUMBENT_USERSOLN:
    case CPX_CALLBACK_MIP_INCUMBENT_MIPSTART:
      return AMPLCBWhere::mipsol;
    default:
      return AMPLCBWhere::notmapped;
    }
  }
  myobj get(int what);
  int getInt(int what)
  {
    int res;
    int status;
    if (what < CPX_CALLBACK_INFO_NODE_SIINF)
    {
      status = CPXgetcallbackinfo(env(), cbdata_, wherefrom_,
        what, &res);
      if (status)
      {
        printf("While getting %d (where=%d)\n", what, wherefrom_);
        printf("ERROR %s\n", model_->error(status).c_str());
      }
      return res;
    }
    throw std::exception("Not supported yet");
  }
  double getDouble(int what)
  {
    double res;
    int status = CPXgetcallbackinfo(env(), cbdata_, wherefrom_,
      what, &res);
    return res;
  }
  virtual myobj getValue(AMPLCBValue::Value v) {
    switch (v)
    {
    case AMPLCBValue::iterations:
      if (wherefrom_ < CPX_CALLBACK_MIP)
        return get(CPX_CALLBACK_INFO_ITCOUNT);
      else
        return get(CPX_CALLBACK_INFO_MIP_ITERATIONS);
    case AMPLCBValue::obj:
      myobj o;
      o.type = 2;
      o.dbl = getObjective();
      return o;
    case AMPLCBValue::pre_delcols:
      return get(CPX_CALLBACK_INFO_PRESOLVE_COLSGONE);
    case AMPLCBValue::pre_delrows:
      return get(CPX_CALLBACK_INFO_PRESOLVE_ROWSGONE);
    case AMPLCBValue::pre_coeffchanged:
      return get(CPX_CALLBACK_INFO_PRESOLVE_COEFFS);
    }
  }
};

} // namespace
#endif // CPLEX_CALLBACK_H_INCLUDE_