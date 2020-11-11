#ifndef GUROBI_CALLBACK_H_INCLUDE_
#define GUROBI_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "gurobi_c.h"

namespace ampls {
namespace grb{ namespace impl{
int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata); 
} }
class GurobiModel;

/**
* Base class for Gurobi callbacks, inherit from this to declare a
* callback to be called at various stages of the solution process.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class GurobiCallback : public impl::BaseCallback {
  friend int grb::impl::callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata);
  friend class GurobiModel;
  void* cbdata_;

protected:
  // Interface
  int doAddCut(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs,
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
  const char* getWhereString();
  /**
  To get the gurobi log message
  */
  const char* getMessage();

  using BaseCallback::getSolutionVector;
  int getSolution(int len, double* sol);
  double getObj();

  // ************** Gurobi specific **************
  /** Get CBdata, useful for calling gurobi c library functions */
  void* getCBData() { return cbdata_; }
  /** * Get the underlying gurobi model pointer */
  GRBmodel* getGRBModel();
  /** Terminate the solution */
  void terminate();
  /** Get an integer attribute (using gurobi C library enumeration to specify what)*/
  int getInt(int what) {
    int res;
    int status = GRBcbget(cbdata_, where_, what, &res);
    if (status)
      throw ampls::AMPLSolverException::format("Error while getting int attribute, code: %d", status);
    return res;
  }
  /** Get a double attribute (using gurobi C library enumeration to specify what)*/
  double getDouble(int what) {
    double res;
    int status = GRBcbget(cbdata_, where_, what, &res);
    if (status)
      throw ampls::AMPLSolverException::format("Error while getting double, code: %d", status);
    return res;
  }
  /** Get a double array attribute (using gurobi C library enumeration to specify what)*/
  std::vector<double> getDoubleArray(int what) {
    int len = model_->getNumVars();
    std::vector<double> res;
    res.resize(len);
    int status = GRBcbget(cbdata_, where_, what, res.data());
    if (status)
      throw ampls::AMPLSolverException::format("Error while getting double attribute, code: %d", status);
    return res;
  }
  /** Set the current solution */
  double setSolution(double* x)
  {
    double obj;
    int status = GRBcbsolution(cbdata_, x, &obj);
    if (status)
      throw ampls::AMPLSolverException::format("Error while setting solution, code: %d", status);
    return obj;
  }

  virtual Where::CBWhere getAMPLWhere() {
    switch (where_)
    {
    case GRB_CB_MESSAGE:
      return Where::MSG;
    case GRB_CB_PRESOLVE:
      return Where::PRESOLVE;
    case GRB_CB_SIMPLEX:
      return Where::LPSOLVE;
    case GRB_CB_MIPNODE:
      return Where::MIPNODE;
    case GRB_CB_MIPSOL:
      return Where::MIPSOL;
    case GRB_CB_MIP:
      return Where::MIP;
    default:
      return Where::NOTMAPPED;
    }
  }
  /** Get a value (using gurobi C library enumeration to specify what)*/
  Variant get(int what);
  
  virtual Variant getValue(Value::CBValue v) {
    switch (v)
    {
    case Value::OBJ:
      return Variant(getObj());
    case Value::MIP_RELATIVEGAP:
      return get(GRB_CB_MIPNODE_REL);
    case Value::PRE_DELCOLS:
      return get(GRB_CB_PRE_COLDEL);
    case Value::PRE_DELROWS:
      return get(GRB_CB_PRE_ROWDEL);
    case Value::PRE_COEFFCHANGED:
      return get(GRB_CB_PRE_COECHG);
    case Value::ITERATIONS:
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
