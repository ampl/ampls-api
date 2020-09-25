#ifndef GUROBI_CALLBACK_H_INCLUDE_
#define GUROBI_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "simpleapi/simpleApi.h"

#include "gurobi_c.h"

namespace ampls {

class GurobiModel;

class GurobiCallback : public impl::BaseCallback {
  friend int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata);
  friend class GurobiModel;
  void* cbdata_;
  GurobiModel* gurobiModel() {
    return (GurobiModel*)model_;
  };
protected:
  // Interface
  int doAddCut(int nvars, const int* vars,
    const double* coeffs, int direction, double rhs,
    int type);

public:
  GurobiCallback() : cbdata_(NULL) {}

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

  virtual CBWhere::Where getAMPLType() {
    switch (where_)
    {
    case GRB_CB_MESSAGE:
      return CBWhere::msg;
    case GRB_CB_PRESOLVE:
      return CBWhere::presolve;
    case GRB_CB_SIMPLEX:
      return CBWhere::lpsolve;
    case GRB_CB_MIPNODE:
      return CBWhere::mipnode;
    case GRB_CB_MIPSOL:
      return CBWhere::mipsol;
    case GRB_CB_MIP:
      return CBWhere::mip;
    default:
      return CBWhere::notmapped;
    }

  }
  Variant get(int what);
  virtual Variant getValue(CBValue::Value v) {
    int grbv;
    Variant result;
    switch (v)
    {
    case CBValue::obj:
      result.type = 2;
      result.dbl = getObjective();
      return result;
    case CBValue::pre_delcols:
      grbv = GRB_CB_PRE_COLDEL;
      break;
    case CBValue::pre_delrows:
      grbv = GRB_CB_PRE_ROWDEL;
      break;
    case CBValue::pre_coeffchanged:
      grbv = GRB_CB_PRE_COECHG;
      break;
    case CBValue::iterations:
      if (where_ == GRB_CB_SIMPLEX)
        grbv = GRB_CB_SPX_ITRCNT;
      if ((where_ >= GRB_CB_MIP) &&
        (where_ >= GRB_CB_MIPNODE))
        grbv = GRB_CB_MIP_ITRCNT;
      if (where_ == GRB_CB_BARRIER)
        grbv = GRB_CB_BARRIER_ITRCNT;
    }
    return get((int)grbv);
  }
};

} // namespace
#endif // GUROBI_CALLBACK_H_INCLUDE_
