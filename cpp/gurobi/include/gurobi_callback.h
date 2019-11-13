#ifndef GUROBI_CALLBACK_H_INCLUDE_
#define GUROBI_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "simpleapi/simpleApi.h"

#include "gurobi_c.h"

class GurobiModel;




class GRBCallback : public BaseCallback {
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
  GRBCallback() : cbwhere_(), cbdata_(NULL)
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

  virtual AMPLCBWhere::Value getAMPLType() {
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
    default:
      return AMPLCBWhere::notmapped;
    }

  }
#ifdef SWIG
  myobj get(int what);
#endif
};


#endif // GUROBI_CALLBACK_H_INCLUDE_
