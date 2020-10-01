#ifndef XPRESS_CALLBACK_H_INCLUDE_
#define XPRESS_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "simpleapi/simpleApi.h"

#include "xprs.h"

namespace ampls
{
class XPRESSModel;
namespace xpress {
namespace impl {
class CBWrap;
}
}
class XPRESSCallback : public impl::BaseCallback {
  char BUFFER[256];

  friend class XPRESSModel;
  friend xpress::impl::CBWrap;
  // Stores the pointer to the XPRESS model being used, as passed from the callback
  XPRSprob prob_;
  // Stores the pointer to the data passed from XPRESS to the callback
  void* object_;

  // Stores the message passed from XPRESS msg function to the callback
  const char* msg_;
  int msgType;

  // Stores obj and solution values when incumbent found
  double objval_;
  double* x_;


protected:
  // Interface
  int doAddCut(int nvars, const int* vars,
    const double* coeffs, int direction, double rhs,
    int type);
public:

  virtual int run() = 0;
  ~XPRESSCallback() {};

  // Interface
  using BaseCallback::getSolutionVector;
  int getSolution(int len, double* sol);
  double getObjective() {
    // TODO if in MIP?
    return getDouble(XPRS_LPOBJVAL);
  }
  const char* getWhere();
  const char* getMessage();

  CBWhere::Where getAMPLType() {
    return static_cast<CBWhere::Where>(where_);
  }
  Variant get(int what);
  int getInt(int what)
  {
    int val;
    XPRSgetintattrib(prob_, what, &val);
    return val;
  }
  double getDouble(int what)
  {
    double val;
    XPRSgetdblattrib(prob_, what, &val);
    return val;
  }
  virtual Variant getValue(CBValue::Value v) {
    throw std::exception("Not supported yet");
  }
};

} // namespace
#endif // XPRESS_CALLBACK_H_INCLUDE_