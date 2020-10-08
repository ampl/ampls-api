#include "xpress_interface.h"
#include "xpress_callback.h"

namespace ampls
{
const char* XPRESSCallback::getMessage() {
  return msg_;
}

int XPRESSCallback::doAddCut(int nvars, const int* vars,
  const double* coeffs, CutDirection::Direction direction, double rhs, int lazy) {

  printCut(nvars, vars, coeffs, direction, rhs);
  char sense;
  switch (direction)
  {
    case CutDirection::eq:
      sense = 'E';
      break;
    case CutDirection::ge:
      sense = 'G';
      break;
    case CutDirection::le:
      sense = 'L';
      break;
    default:
      throw AMPLSolverException("Unexpected cut direction");
  }
  throw std::exception("Not implemented yet");
}

int XPRESSCallback::getSolution(int len, double* sol) {
  int nvars = model_->getNumVars();
  if (len < nvars)
    throw AMPLSolverException::format("Must allocate an array of at least %d elements.", nvars);
  if (where_ == (int)xpress::impl::XPRESSWhere::prenode)
    return XPRSgetpresolvesol(prob_, sol, NULL, NULL, NULL);
  if (where_ == (int)xpress::impl::XPRESSWhere::optnode)
    return XPRSgetlpsol(prob_, sol, NULL, NULL, NULL);

  throw ampls::AMPLSolverException("Cannot get the solution vector in this stage.");
}

using xpress::impl::XPRESSWhere;

const char* XPRESSCallback::getWhere()
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

} // namespace