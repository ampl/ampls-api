#ifndef COPT_CALLBACK_H_INCLUDE_
#define COPT_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "copt.h"

namespace ampls {
namespace impl{ namespace copt {
  int COPT_CALL copt_callback_wrapper(copt_prob* prob, void* cbdata, int cbctx, void* userdata);
  void COPT_CALL copt_log_callback_wrapper(char* msg, void* userdata);
} }
class CoptModel;

/**
* Base class for Copt callbacks, inherit from this to declare a
* callback to be called at various stages of the solution process.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class CoptCallback : public impl::BaseCallback {
  friend int impl::copt::copt_callback_wrapper(copt_prob* prob, void* cbdata, int cbctx, void* userdata);
  friend void impl::copt::copt_log_callback_wrapper(char* msg, void* userdata);
  friend class CoptModel;
  void* cbdata_;
  static char toCOPTSense(ampls::CutDirection::Direction dir)
  {
    switch (dir)
    {
    case CutDirection::EQ:
      return COPT_EQUAL;
    case CutDirection::GE:
      return COPT_GREATER_EQUAL;
    case CutDirection::LE:
      return COPT_LESS_EQUAL;
    }
    throw std::runtime_error("Unexpected CutDirection value");
  }


  Variant getValueImpl(Value::CBValue v);
protected:
  // Interface
  int doAddCut(const ampls::Constraint& c, int type, 
    void* additionalParams=nullptr);

public:

  CoptCallback() : cbdata_(NULL) {}
  
  virtual int run() = 0;
  /**
  Get a string description of where the callback was called from
  */
  const char* getWhereString();
  /**
  To get the copt log message
  */
  const char* getMessage();

  using BaseCallback::getSolutionVector;
  using BaseCallback::getWhere;
  int getSolution(int len, double* sol);
  double getObj();

  // ************** Copt specific **************
  /** Get CBdata, useful for calling copt c library functions */
  void* getCBData() { return cbdata_; }
  /** * Get the underlying copt model pointer */
  copt_prob* getCOPTModel();
  /** Terminate the solution */
  void terminate();
  /** Get a double attribute (using copt C library enumeration to specify what)*/
  double getDouble(const char* cbinfo) {
    double res;
    int status = COPT_GetCallbackInfo(cbdata_, cbinfo, &res);
    if (status)
      throw ampls::AMPLSolverException::format("Error while getting double, code: %d", status);
    return res;
  }
  /** Set the current solution */
  double setSolution(double* x)
  {
    double obj;
    int status = COPT_AddCallbackSolution(cbdata_, x, &obj);
    if (status)
      throw ampls::AMPLSolverException::format("Error while setting solution, code: %d", status);
    return obj;
  }

   Where::CBWhere getAMPLWhereImpl() {
    switch (where_)
    {

    case COPT_CBCONTEXT_MIPRELAX:
      return Where::LPSOLVE;
    case COPT_CBCONTEXT_MIPNODE:
      return Where::MIPNODE;
    case COPT_CBCONTEXT_MIPSOL:
    case COPT_CBCONTEXT_INCUMBENT:
      return Where::MIPSOL;
    default:
      return Where::NOTMAPPED;
    }
  }
  /** Get a value (using copt C library enumeration to specify what)*/
  Variant get(const char* what);
  

  int setHeuristicSolution(int nvars, const int* indices, const double* values);

  std::vector<double> getValueArray(Value::CBValue v);
};

} // namespace
#endif // COPT_CALLBACK_H_INCLUDE_
