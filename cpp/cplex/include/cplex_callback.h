#ifndef CPLEX_CALLBACK_H_INCLUDE_
#define CPLEX_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "ilcplex/cplex.h"

namespace ampls
{
namespace cpx { namespace impl { class CBWrap; } }
class CPLEXModel;

/**
* Base class for CPLEX callbacks, inherit from this to declare a
* callback to be called at various stages of the solution process.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
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
  CPXCENVptr getCPXENV() { return env_; }
  void* getCBData() { return cbdata_; }
public:

  virtual int run() = 0;
  ~CPLEXCallback() {};

  // Interface
  using BaseCallback::getSolutionVector;
  int getSolution(int len, double* sol);
  double getObj();
  using BaseCallback::getWhere;
  const char* getWhereString();
  const char* getMessage();

  Where::CBWhere getAMPLWhere() {
    switch (getWhere())
    {
    case -1:
      return  Where::MSG;
    case CPX_CALLBACK_PRESOLVE:
      return Where::PRESOLVE;
    case CPX_CALLBACK_PRIMAL:
    case CPX_CALLBACK_DUAL:
    case CPX_CALLBACK_BARRIER:
      return Where::LPSOLVE;
    //case CPX_CALLBACK_MIP_NODE:
      // For user cuts
    case CPX_CALLBACK_MIP_CUT_LOOP:
    case CPX_CALLBACK_MIP_CUT_LAST:
      return Where::MIPNODE;
    //case CPX_CALLBACK_MIP_INCUMBENT_NODESOLN:
    //case CPX_CALLBACK_MIP_INCUMBENT_HEURSOLN:
    //case CPX_CALLBACK_MIP_INCUMBENT_USERSOLN:
    // For lazy constraints
    case CPX_CALLBACK_MIP_CUT_FEAS:
    case CPX_CALLBACK_MIP_CUT_UNBD:
      return Where::MIPSOL;
    default:
      return Where::NOTMAPPED;
    }
  }
  /** Get a  value at this stage of the solution process */
  Variant get(int what);
  /** Get an integer value at this stage of the solution process*/
  int getInt(int what)
  {
    int res;
    int status;
    if (what < CPX_CALLBACK_INFO_NODE_SIINF)
    {
      status = CPXgetcallbackinfo(getCPXENV(), cbdata_, where_,
        what, &res);
      if (status)
      {
        printf("While getting %d (where=%d)\n", what, where_);
        printf("ERROR %s\n", model_->error(status).c_str());
      }
      return res;
    }
    throw std::runtime_error("Not supported yet");
  }

  /** Get a double value at this stage of the solution process*/
  double getDouble(int what)
  {
    double res;
    int status = CPXgetcallbackinfo(getCPXENV(), cbdata_, where_,
      what, &res);
    return res;
  }

  virtual Variant getValue(Value::CBValue v) {
    switch (v)
    {
    case Value::ITERATIONS:
      if (where_ < CPX_CALLBACK_MIP)
        return get(CPX_CALLBACK_INFO_ITCOUNT);
      else
        return get(CPX_CALLBACK_INFO_MIP_ITERATIONS);
    case Value::OBJ:
      return Variant(getObj());
    case Value::PRE_DELCOLS:
      return get(CPX_CALLBACK_INFO_PRESOLVE_COLSGONE);
    case Value::PRE_DELROWS:
      return get(CPX_CALLBACK_INFO_PRESOLVE_ROWSGONE);
    case Value::PRE_COEFFCHANGED:
      return get(CPX_CALLBACK_INFO_PRESOLVE_COEFFS);
    case Value::MIP_RELATIVEGAP:
      return get(CPX_CALLBACK_INFO_MIP_REL_GAP);
    default: throw AMPLSolverException("Specified value unknown.");
    }
  }
};

} // namespace
#endif // CPLEX_CALLBACK_H_INCLUDE_