#ifndef XPRESS_CALLBACK_H_INCLUDE_
#define XPRESS_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "xprs.h"

namespace ampls
{
class XPRESSModel;

namespace impl {
  namespace xpress {
class XPRSCBWrap;
enum class XPRESSWhere
{
  message,
  intsol,
  infnode,
  newnode,
  nodecutoff,
  chgbranch,
  prenode,
  optnode
};
}
}

/**
* Base class for XPRESS callbacks, inherit from this to declare a
* callback to be called at various stages of the solution process.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class XPRESSCallback : public impl::BaseCallback {
  char BUFFER[256];

  friend class XPRESSModel;
  friend impl::xpress::XPRSCBWrap;
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

  // Stores the feasibility status when adding user cut via callback
  int feas_;

  int preintsol_;

  static const char toXPRESSRowType[3];

protected:
  bool isPreIntSol() {
    return preintsol_!=0;
  }
  // Interface
  int doAddCut(const ampls::Constraint& c, int type, 
    void* additionalParams=nullptr);
public:

  virtual int run() = 0;
  ~XPRESSCallback() {};


  std::vector<double> getSolutionVector();
  int getSolution(int len, double* sol);
  double getObj() {
    if (getInt(XPRS_ORIGINALMIPENTS) > 0)
      return getDouble(XPRS_MIPOBJVAL);
    else
      return getDouble(XPRS_LPOBJVAL);
  }
  const char* getWhereString();
  const char* getMessage();

  Where::CBWhere getAMPLWhereImpl() {
    switch (static_cast<impl::xpress::XPRESSWhere>(where_))
    {
    case impl::xpress::XPRESSWhere::message:
      return Where::MSG;
    case impl::xpress::XPRESSWhere::intsol:
      return Where::MIPSOL;
    case impl::xpress::XPRESSWhere::optnode:
    case impl::xpress::XPRESSWhere::newnode:
      return Where::MIPNODE;
    default:
      return Where::NOTMAPPED;
    }
  }

  Variant getValueImpl(Value::CBValue v);

  // ************ XPRESS specific ************
  /** Get an attribute variant */
  Variant get(int what);

  /** Get an integer attribute */
  int getInt(int what)
  {
    int val;
    XPRSgetintattrib(prob_, what, &val);
    return val;
  }
  /** Get a double attribute */
  double getDouble(int what)
  {
    double val;
    XPRSgetdblattrib(prob_, what, &val);
    return val;
  }
  /** Get the underlying XPRSprob structure*/
    XPRSprob getXPRSprob() {
    return prob_;
  }

    int setHeuristicSolution(int nvars, const int* indices, const double* values);

    std::vector<double> getValueArray(Value::CBValue v);
};

} // namespace
#endif // XPRESS_CALLBACK_H_INCLUDE_