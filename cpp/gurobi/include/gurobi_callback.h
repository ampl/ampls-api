#ifndef GUROBI_CALLBACK_H_INCLUDE_
#define GUROBI_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "simpleapi/simpleApi.h"

#include "gurobi_c.h"

namespace ampls {
namespace grb{ namespace impl{
int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata); 
} }
class GurobiModel;

class GurobiCallback : public impl::BaseCallback {
  friend int grb::impl::callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata);
  friend class GurobiModel;
  void* cbdata_;
  GurobiModel* gurobiModel() {
    return (GurobiModel*)model_;
  };
protected:
  // Interface
  int doAddCut(int nvars, const int* vars,
    const double* coeffs, CutDirection direction, double rhs,
    int type);

public:
  GurobiCallback() : cbdata_(NULL) {}
  /**
  * Get where the callback is called from in Gurobi_C library metrics
  */
  int where() {
    return where_;
  }
  virtual int run() = 0;
  /**
  Get a string description of where the callback was called from
  */
  const char* getWhere();
  /**
  To get the gurobi log message
  */
  const char* getMessage();

  // Interface
  using BaseCallback::getSolutionVector;
  int getSolution(int len, double* sol);
  double getObjective();

  void terminate();

  int getInt(int what) {
    int res;
    int status = GRBcbget(cbdata_, where_, what, &res);
    return res;
  }
  double getDouble(int what) {
    double res;
    int status = GRBcbget(cbdata_, where_, what, &res);
    return res;
  }

  virtual Where getAMPLType() {
    switch (where_)
    {
    case GRB_CB_MESSAGE:
      return Where::msg;
    case GRB_CB_PRESOLVE:
      return Where::presolve;
    case GRB_CB_SIMPLEX:
      return Where::lpsolve;
    case GRB_CB_MIPNODE:
      return Where::mipnode;
    case GRB_CB_MIPSOL:
      return Where::mipsol;
    case GRB_CB_MIP:
      return Where::mip;
    default:
      return Where::notmapped;
    }

  }
  Variant get(int what);
  virtual Variant getValue(Value v) {
    switch (v)
    {
    case Value::obj:
      return Variant(getObjective());
    case Value::mip_relativegap:
      return get(GRB_CB_MIPNODE_REL);
    case Value::pre_delcols:
      return get(GRB_CB_PRE_COLDEL);
    case Value::pre_delrows:
      return get(GRB_CB_PRE_ROWDEL);
    case Value::pre_coeffchanged:
      return get(GRB_CB_PRE_COECHG);
    case Value::iterations:
      if (where_ == GRB_CB_SIMPLEX)
        return get(GRB_CB_SPX_ITRCNT);
      if ((where_ >= GRB_CB_MIP) &&
        (where_ >= GRB_CB_MIPNODE))
        return get(GRB_CB_MIP_ITRCNT);
      if (where_ == GRB_CB_BARRIER)
        return get(GRB_CB_BARRIER_ITRCNT);
    default:
      throw AMPLSolverException("Specified value unknown.");
    }
  }
};

} // namespace
#endif // GUROBI_CALLBACK_H_INCLUDE_
