#include "xpress_interface.h"
#include "xpress_callback.h"

namespace ampls
{

const char XPRESSCallback::toXPRESSRowType[3] = { 'E', 'G', 'L' };

const char* XPRESSCallback::getMessage() {
  return msg_;
}

int XPRESSCallback::doAddCut(const ampls::Constraint& c, int type) {
  int cutType[1] = { 1 };
  char sense[1] = { toXPRESSRowType[(int)c.sense()] };
  double rhs[1] = { c.rhs() };
  int start[2] = { 0, c.indices().size() };

  printf("I have %d vars and %d cons\n", model_->getNumVars(), model_->getNumCons());
  return XPRSaddcuts(prob_, 1, cutType, sense, rhs, start,
    c.indices().data(), c.coeffs().data());
}

int XPRESSCallback::getSolution(int len, double* sol) {
  int nvars = model_->getNumVars();
  if (len < nvars)
    throw AMPLSolverException::format("Must allocate an array of at least %d elements.", nvars);
  if (where_ == (int)xpress::impl::XPRESSWhere::prenode)
    return XPRSgetpresolvesol(prob_, sol, NULL, NULL, NULL);
  if ((where_ == (int)xpress::impl::XPRESSWhere::optnode) ||
    (where_ == (int)xpress::impl::XPRESSWhere::intsol))
    return XPRSgetlpsol(prob_, sol, NULL, NULL, NULL);

  throw ampls::AMPLSolverException("Cannot get the solution vector in this stage.");
}

using xpress::impl::XPRESSWhere;

const char* XPRESSCallback::getWhereString()
{
  xpress::impl::XPRESSWhere proxy = (xpress::impl::XPRESSWhere)where_;
  switch (proxy)
  {
  case XPRESSWhere::message: return "message";
  case XPRESSWhere::intsol: return "intsol";
  case XPRESSWhere::chgnode: return "chgnode";
  case XPRESSWhere::infnode: return "infnode";
  case XPRESSWhere::nodecutoff: return "nodecutoff";
  case XPRESSWhere::chgbranch: return "chgbranch";
  case XPRESSWhere::prenode: return "prenode";
  case XPRESSWhere::optnode: return "optnode";
  }
  throw AMPLSolverException("Where not mapped!");
}

Variant XPRESSCallback::get(int what)
{
  Variant r = Variant();
  if ((what >= XPRS_MATRIXNAME) && (what <= XPRS_UUID))
  {
    XPRSgetstrattrib(prob_, what, BUFFER);
    r.type = 2;
    r.str = BUFFER;
    return r;
  }
  if ( ((what >= XPRS_LPOBJVAL) && (what <= XPRS_MAXKAPPA)) ||
    ((what >= XPRS_BARPRIMALOBJ) && (what <= XPRS_BARCGAP))
    )
  {
    r.type = 2;
    r.dbl = getDouble(what);
    return r;
  }
  if ( ((what >= XPRS_ROWS) && (what <= XPRS_COMPUTEEXECUTIONS)) ||
    ((what >= XPRS_BARITER) && (what <= XPRS_BARCROSSOVER)) )
  {
    r.type = 1;
    r.integer= getInt(what);
    return r;
  }
  throw AMPLSolverException("Invalid parameter");
}

Variant XPRESSCallback::getValue(Value::CBValue v) {
  switch (v)
  {
  case Value::PRE_DELCOLS:
    return Variant(getInt(XPRS_ORIGINALCOLS) - getInt(XPRS_COLS));
  case Value::PRE_DELROWS:
    return Variant(getInt(XPRS_ORIGINALROWS) - getInt(XPRS_ROWS));
  case Value::PRE_COEFFCHANGED:
    return Variant(0);
  case Value::ITERATIONS:
    return Variant(getInt(XPRS_BARITER));
  case Value::OBJ:
    return Variant(getDouble(XPRS_MIPBESTOBJVAL));
    //return Variant(getDouble(XPRS_LPOBJVAL));
  case Value::RUNTIME:
    return Variant(((double)clock() - ((XPRESSModel*)model_)->tStart_) / CLOCKS_PER_SEC);
  }
  throw std::runtime_error("Not supported yet");
  return Variant(); // silence gcc warning
}

int XPRESSCallback::setHeuristicSolution(int nvars, const int* indices, const double* values) {
  return XPRSaddmipsol(prob_, nvars, values, indices, NULL);

}

} // namespace