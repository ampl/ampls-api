#include "xpress_interface.h"
#include "xpress_callback.h"

namespace ampls
{

const char XPRESSCallback::toXPRESSRowType[3] = { 'E', 'G', 'L' };

const char* XPRESSCallback::getMessage() {
  return msg_;
}

int XPRESSCallback::doAddCut(const ampls::Constraint& c, int type, 
  void* additionalParams) {
  if (!preintsol_) {
    int cutType = 1;
    char sense = toXPRESSRowType[(int)c.sense()];
    int size = (int)c.indices().size();;
    std::vector<int> indices(size);
    std::vector<double> coeffs(size);
    int nnewcoffs;
    double rhs;
    int status;
    int max = size;
    XPRSpresolverow(prob_, sense, size,
      c.indices().data(), c.coeffs().data(), c.rhs(), max, &nnewcoffs,
      indices.data(), coeffs.data(), &rhs, &status);

    if (status >= 0) {
      int mtype = 0, mstart[2] = { 0, nnewcoffs };
      int status = XPRSaddcuts(prob_, 1, &cutType, &sense, &rhs, mstart,
        indices.data(), coeffs.data());

    }
  }
  else // Here if we're trying to add a cut in preintsol. 
    feas_ = 1;
  return 0;
}

int XPRESSCallback::getSolution(int len, double* sol) {
  int nvars = getInt(XPRS_COLS);
  if (len < nvars)
    throw AMPLSolverException::format("Must allocate an array of at least %d elements.", nvars);
  if (where_ == (int)impl::xpress::XPRESSWhere::prenode)
    return XPRSgetpresolvesol(prob_, sol, NULL, NULL, NULL);
  if ((where_ == (int)impl::xpress::XPRESSWhere::optnode) ||
    (where_ == (int)impl::xpress::XPRESSWhere::intsol))
    return XPRSgetlpsol(prob_, sol, NULL, NULL, NULL);

  throw ampls::AMPLSolverException("Cannot get the solution vector in this stage.");
}


std::vector<double> XPRESSCallback::getSolutionVector() {
  int len = getInt(XPRS_COLS);
  std::vector<double> res(len);
  int s;
  try {
    s = getSolution(len, res.data());
  }
  catch (...)
  {
    return res;
  }
  return res;
}

using impl::xpress::XPRESSWhere;

const char* XPRESSCallback::getWhereString()
{
  impl::xpress::XPRESSWhere proxy = (impl::xpress::XPRESSWhere)where_;
  switch (proxy)
  {
  case XPRESSWhere::message: return "message";
  case XPRESSWhere::intsol: return "intsol";
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

Variant XPRESSCallback::getValueImpl(Value::CBValue v) {
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
    return Variant(getDouble(XPRS_MIPOBJVAL)); 
  case Value::RUNTIME:
    return Variant(getDouble(XPRS_TIME));
  case Value::MIP_OBJBOUND:
    return Variant(getDouble(XPRS_BESTBOUND));
  case Value::MIP_RELATIVEGAP:
    return impl::calculateRelMIPGAP(getObj(),
      getValueImpl(Value::MIP_OBJBOUND).dbl);
  case Value::N_COLS:
    return Variant(getInt(XPRS_ORIGINALCOLS));
  case Value::N_ROWS:
    return Variant(getInt(XPRS_ORIGINALROWS));
  case Value::MIP_NODES:
    return Variant(getInt(XPRS_CURRENTNODE));
  }
  throw std::runtime_error("Not supported yet");
  return Variant(); // silence gcc warning
}

int XPRESSCallback::setHeuristicSolution(int nvars, const int* indices, const double* values) {
  return XPRSaddmipsol(prob_, nvars, values, indices, NULL);
}

std::vector<double> XPRESSCallback::getValueArray(Value::CBValue v) {
  switch (v)
  {
    case Value::MIP_SOL_RELAXED:
        return getSolutionVector();
  }
  throw std::runtime_error("Not supported yet");
}

} // 