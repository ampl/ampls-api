#include "xpress_interface.h"
#include "xpress_callback.h"

namespace ampls
{

const char XPRESSCallback::toXPRESSRowType[3] = { 'E', 'G', 'L' };

const char* XPRESSCallback::getMessage() {
  return msg_;
}

int XPRESSCallback::doAddCut(const ampls::Constraint& c, int type) {
  int res = 0;
  if (!preintsol_) {
    int cutType = { 1 };
    char sense = toXPRESSRowType[(int)c.sense()];
    int size = (int)c.indices().size();;
    std::vector<int> indices(size);
    std::vector<double> coeffs(size);
    int nnewcoffs;
    double rhs;
    int status;
    // TODO: Num cols
    int max = size;
    XPRSpresolverow(prob_, sense, size,
      c.indices().data(), c.coeffs().data(), c.rhs(), max, &nnewcoffs,
      indices.data(), coeffs.data(), &rhs, &status);

    if (status >= 0) {
      int mtype = 0, mstart[2];
      mstart[0] = 0; mstart[1] = nnewcoffs;
      XPRSaddcuts(prob_, 1, &mtype, &sense, &rhs, mstart, indices.data(), coeffs.data());

    }
  }
  if (res == 0)
    feas_ = 1;
  return res;
}

int XPRESSCallback::getSolution(int len, double* sol) {
  int nvars = model_->getNumVars();
  if (len < nvars)
    throw AMPLSolverException::format("Must allocate an array of at least %d elements.", nvars);
  if (where_ == (int)impl::xpress::XPRESSWhere::prenode)
    return XPRSgetpresolvesol(prob_, sol, NULL, NULL, NULL);
  if ((where_ == (int)impl::xpress::XPRESSWhere::optnode) ||
    (where_ == (int)impl::xpress::XPRESSWhere::intsol))
    return XPRSgetlpsol(prob_, sol, NULL, NULL, NULL);

  throw ampls::AMPLSolverException("Cannot get the solution vector in this stage.");
}

using impl::xpress::XPRESSWhere;

const char* XPRESSCallback::getWhereString()
{
  impl::xpress::XPRESSWhere proxy = (impl::xpress::XPRESSWhere)where_;
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
    return Variant(getDouble(XPRS_MIPOBJVAL));
    //return Variant(getDouble(XPRS_LPOBJVAL));
  case Value::RUNTIME:
    return Variant(((double)clock() - ((XPRESSModel*)model_)->tStart_) / CLOCKS_PER_SEC);
  case Value::MIP_OBJBOUND:
    return Variant(getDouble(XPRS_BESTBOUND));
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
}

} // 