#ifndef CPLEX_CALLBACK_H_INCLUDE_
#define CPLEX_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "simpleapi/simpleApi.h"

#include "ilcplex/cplex.h"

namespace ampls
{
namespace cpx { namespace impl { class CBWrap; } }
class CPLEXModel;

class CPLEXCallback : public impl::BaseCallback {
  char CODE[60];
  friend class CPLEXModel;
  friend class  cpx::impl::CBWrap;
  // Callback data
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
    const double* coeffs, CutDirection::Direction direction, double rhs,
    int type);
  CPXCENVptr env() { return env_; }
  void* cbdata() { return cbdata_; }
public:

  virtual int run() = 0;
  ~CPLEXCallback() {};

  // Interface
  using BaseCallback::getSolutionVector;
  int getSolution(int len, double* sol);
  double getObjective();
  const char* getWhere();
  const char* getMessage();

  Where::CBWhere getAMPLType() {

    switch (where_)
    {
    case -1:
      return  Where::msg;
    case CPX_CALLBACK_PRESOLVE:
      return Where::presolve;
    case CPX_CALLBACK_PRIMAL:
    case CPX_CALLBACK_DUAL:
    case CPX_CALLBACK_BARRIER:
      return Where::lpsolve;
    case CPX_CALLBACK_MIP_NODE:
    case CPX_CALLBACK_MIP_CUT_FEAS:
    case CPX_CALLBACK_MIP_CUT_UNBD:
    case CPX_CALLBACK_MIP_CUT_LOOP:
    case CPX_CALLBACK_MIP_CUT_LAST:
      return Where::mipnode;
    case CPX_CALLBACK_MIP_INCUMBENT_NODESOLN:
    case CPX_CALLBACK_MIP_INCUMBENT_HEURSOLN:
    case CPX_CALLBACK_MIP_INCUMBENT_USERSOLN:
    case CPX_CALLBACK_MIP_INCUMBENT_MIPSTART:
      return Where::mipsol;
    default:
      return Where::notmapped;
    }
  }
  Variant get(int what);
  int getInt(int what)
  {
    int res;
    int status;
    if (what < CPX_CALLBACK_INFO_NODE_SIINF)
    {
      status = CPXgetcallbackinfo(env(), cbdata_, where_,
        what, &res);
      if (status)
      {
        printf("While getting %d (where=%d)\n", what, where_);
        printf("ERROR %s\n", model_->error(status).c_str());
      }
      return res;
    }
    throw std::exception("Not supported yet");
  }
  double getDouble(int what)
  {
    double res;
    int status = CPXgetcallbackinfo(env(), cbdata_, where_,
      what, &res);
    return res;
  }
  virtual Variant getValue(Value::CBValue v) {
    switch (v)
    {
    case Value::iterations:
      if (where_ < CPX_CALLBACK_MIP)
        return get(CPX_CALLBACK_INFO_ITCOUNT);
      else
        return get(CPX_CALLBACK_INFO_MIP_ITERATIONS);
    case Value::obj:
      return Variant(getObjective());
    case Value::pre_delcols:
      return get(CPX_CALLBACK_INFO_PRESOLVE_COLSGONE);
    case Value::pre_delrows:
      return get(CPX_CALLBACK_INFO_PRESOLVE_ROWSGONE);
    case Value::pre_coeffchanged:
      return get(CPX_CALLBACK_INFO_PRESOLVE_COEFFS);
    case Value::mip_relativegap:
      return get(CPX_CALLBACK_INFO_MIP_REL_GAP);
    default: throw AMPLSolverException("Specified value unknown.");
    }
  }
};

} // namespace
#endif // CPLEX_CALLBACK_H_INCLUDE_