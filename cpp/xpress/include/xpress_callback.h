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
enum class XPRESSWhere
{
  message,
  intsol,
  chgnode,
  infnode,
  nodecutoff,
  chgbranch,
  prenode,
  optnode
};
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
    const double* coeffs, CutDirection::Direction direction, double rhs,
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

  Where::CBWhere getAMPLType() {
    switch (static_cast<xpress::impl::XPRESSWhere>(where_))
    {
    case xpress::impl::XPRESSWhere::message:
      return Where::msg;
    case xpress::impl::XPRESSWhere::intsol:
      return Where::mipsol;
    case xpress::impl::XPRESSWhere::chgnode:
      return Where::mipnode;
    default:
      return Where::notmapped;
    }
    
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
  virtual Variant getValue(Value::CBValue v) {
    switch (v)
    {
    case Value::pre_delcols:
      return Variant(getInt(XPRS_ORIGINALCOLS) - getInt(XPRS_COLS));
    case Value::pre_delrows:
      return Variant(getInt(XPRS_ORIGINALROWS) - getInt(XPRS_ROWS));
    case Value::pre_coeffchanged:
      return Variant(0);
    }
    throw std::exception("Not supported yet");
  }
};

} // namespace
#endif // XPRESS_CALLBACK_H_INCLUDE_