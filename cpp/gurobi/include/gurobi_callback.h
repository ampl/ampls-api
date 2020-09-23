#ifndef GUROBI_CALLBACK_H_INCLUDE_
#define GUROBI_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "simpleapi/simpleApi.h"

#include "gurobi_c.h"

namespace ampl {

class GurobiModel;

class GurobiCallback : public impl::BaseCallback {
  friend int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata);
  friend class GurobiModel;
  int cbwhere_;
  void* cbdata_;

  GurobiModel* gurobiModel() {
    return (GurobiModel*)model_;
  };
protected:
  // Interface
  int doAddCut(int nvars, const int* vars,
    const double* coeffs, char direction, double rhs,
    int type);

public:
  GurobiCallback() : cbwhere_(), cbdata_(NULL)
  {}

  virtual int run(int whereFrom) = 0;
  /**
  Get a string description of where the callback was called from
  */
  const char* getWhere(int where);
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
    int status = GRBcbget(cbdata_, cbwhere_, what, &res);
    return res;
  }
  double getDouble(int what) {
    double res;
    int status = GRBcbget(cbdata_, cbwhere_, what, &res);
    return res;
  }

  virtual AMPLCBWhere::Where getAMPLType() {
    switch (cbwhere_)
    {
    case GRB_CB_MESSAGE:
      return AMPLCBWhere::msg;
    case GRB_CB_PRESOLVE:
      return AMPLCBWhere::presolve;
    case GRB_CB_SIMPLEX:
      return AMPLCBWhere::lpsolve;
    case GRB_CB_MIPNODE:
      return AMPLCBWhere::mipnode;
    case GRB_CB_MIPSOL:
      return AMPLCBWhere::mipsol;
    case GRB_CB_MIP:
      return AMPLCBWhere::mip;
    default:
      return AMPLCBWhere::notmapped;
    }

  }
  myobj get(int what);
  virtual myobj getValue(AMPLCBValue::Value v) {
    int grbv;
    myobj result;
    switch (v)
    {
    case AMPLCBValue::obj:
      result.type = 2;
      result.dbl = getObjective();
      return result;
    case AMPLCBValue::pre_delcols:
      grbv = GRB_CB_PRE_COLDEL;
      break;
    case AMPLCBValue::pre_delrows:
      grbv = GRB_CB_PRE_ROWDEL;
      break;
    case AMPLCBValue::pre_coeffchanged:
      grbv = GRB_CB_PRE_COECHG;
      break;
    case AMPLCBValue::iterations:
      if (cbwhere_ == GRB_CB_SIMPLEX)
        grbv = GRB_CB_SPX_ITRCNT;
      if ((cbwhere_ >= GRB_CB_MIP) &&
        (cbwhere_ >= GRB_CB_MIPNODE))
        grbv = GRB_CB_MIP_ITRCNT;
      if (cbwhere_ == GRB_CB_BARRIER)
        grbv = GRB_CB_BARRIER_ITRCNT;
    }
    return get((int)grbv);
  }
};

} // namespace
#endif // GUROBI_CALLBACK_H_INCLUDE_
