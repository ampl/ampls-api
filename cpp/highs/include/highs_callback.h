#ifndef HIGHS_CALLBACK_H_INCLUDE_
#define HIGHS_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "interfaces/highs_c_api.h"
#include "lp_data/HConst.h"

namespace ampls {
namespace impl{ namespace highs {
  void highs_callback_wrapper(int, const char*,
    const HighsCallbackDataOut*,
    HighsCallbackDataIn*, void*);
} }
class HighsModel;

/**
* Base class for Highs callbacks, inherit from this to declare a
* callback to be called at various stages of the solution process.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class HighsCallback : public impl::BaseCallback {
  friend  void impl::highs::highs_callback_wrapper(int where,
    const char* message, const HighsCallbackDataOut* dataout,
    HighsCallbackDataIn* datain, void* userdata);

  friend class HighsModel;
  const HighsCallbackDataOut* cbdata_;
  const char* msg_;
  Variant getValueImpl(Value::CBValue v);
protected:
  // Interface
  int doAddCut(const ampls::Constraint& c, int type) {
    throw ampls::AMPLSolverException("HiGHS does not support lazy constraints/user cuts yet");
  }
public:

  HighsCallback() : cbdata_(NULL) {}
  
  virtual int run() = 0;
  /**
  Get a string description of where the callback was called from
  */
  const char* getWhereString();
  /**
  To get the highs log message
  */
  const char* getMessage();

  using BaseCallback::getSolutionVector;
  using BaseCallback::getWhere;
  int getSolution(int len, double* sol);
  double getObj();

  // ************** Highs specific **************
  /** * Get the underlying highs model pointer */
  void* getHighsModel();
  double setSolution(double* x)
  {
    throw ampls::AMPLSolverException::format("Not supported by HiGHS");
  }
  std::vector<double> getValueArray(Value::CBValue v);
  virtual Where::CBWhere getAMPLWhere() {
    switch (where_)
    {
      
    case kCallbackLogging:
    case kCallbackMipLogging:
      return Where::MSG;
    case kCallbackMipSolution:
    case kCallbackMipImprovingSolution:
      return Where::MIPSOL;
    case kHighsCallbackMipInterrupt:
      return Where::MIP;
    case kCallbackSimplexInterrupt:
    case kHighsCallbackIpmInterrupt:
      return Where::LPSOLVE;
    default:
      return Where::NOTMAPPED;
    }
  }

  int setHeuristicSolution(int nvars, const int* indices, const double* values)
  {
    // TODO
    return 0;
  }

 // std::vector<double> getValueArray(Value::CBValue v);
};

} // namespace
#endif // HIGHS_CALLBACK_H_INCLUDE_
