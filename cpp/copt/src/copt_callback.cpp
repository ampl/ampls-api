#include "copt_callback.h"
#include "copt_interface.h"

namespace ampls
{
const char* CoptCallback::getWhereString()
{
  switch (where_)
  {
  case ampls::Where::MSG: return "ampls::Where::MSG";
  case COPT_CBCONTEXT_MIPRELAX: return "COPT_CBCONTEXT_MIPRELAX";
  case COPT_CBCONTEXT_INCUMBENT: return "COPT_CBCONTEXT_INCUMBENT";
  case COPT_CBCONTEXT_MIPNODE: return "COPT_CBCONTEXT_MIPNODE";
  case COPT_CBCONTEXT_MIPSOL: return "COPT_CBCONTEXT_MIPSOL";
  default:
    return "Unexpected callback status.";
  }
}

Variant CoptCallback::get(const char* what)
{
  Variant r = Variant();
  return getDouble(what);
}

void CoptCallback::terminate() {
  COPT_Interrupt(getCOPTModel());
}
copt_prob* CoptCallback::getCOPTModel() {
    return ((CoptModel*)model_)->getCOPTmodel();
  };
const char* CoptCallback::getMessage()
{
  char* msg;
  if (where_ == ampls::Where::MSG)
    return (char*)cbdata_;
  else
    throw ampls::AMPLSolverException("Cannot get message outside of a log callback.");
}

int CoptCallback::doAddCut(const ampls::Constraint& c, int lazy, 
  void* additionalParams) {
  char sense = toCOPTSense(c.sense());
  if (lazy)
  {
    return COPT_AddCallbackLazyConstr(cbdata_,c.indices().size(), c.indices().data(),
      c.coeffs().data(), sense, c.rhs());
  }
  else
  {
    return COPT_AddCallbackUserCut(cbdata_, c.indices().size(), c.indices().data(),
      c.coeffs().data(), sense, c.rhs());
  }
}

int CoptCallback::getSolution(int len, double* sol)
{
  if (len != model_->getNumVars())
    throw ampls::AMPLSolverException("COPT only supports full solution vectors retrieval.");
  if (where_ == COPT_CBCONTEXT_MIPRELAX)
    COPT_GetCallbackInfo(cbdata_, COPT_CBINFO_RELAXSOLUTION, sol);
  if (where_ == COPT_CBCONTEXT_MIPSOL)
    COPT_GetCallbackInfo(cbdata_, COPT_CBINFO_MIPCANDIDATE, sol);
  return 0;
}

double CoptCallback::getObj()
{
  const char* flag;
  switch (where_)
  {
  case COPT_CBCONTEXT_MIPRELAX:
    flag = COPT_CBINFO_INCUMBENT;
    break;
  case COPT_CBCONTEXT_MIPSOL:
    flag = COPT_CBINFO_BESTOBJ;
    break;
  default:
    throw ampls::AMPLSolverException("Cannot get objective value from here!");
  }
  return getDouble(flag);
}


Variant  CoptCallback::getValueImpl(Value::CBValue v) {
  switch (v)
  {
  case Value::OBJ:
    return Variant(getObj());
  case Value::MIP_RELATIVEGAP:
    return impl::calculateRelMIPGAP(getDouble(COPT_CBINFO_BESTOBJ),
      getDouble(COPT_CBINFO_BESTBND));
  case Value::MIP_OBJBOUND:
    return getDouble(COPT_CBINFO_BESTBND);
  case Value::N_COLS:
    return model_->getNumVars();
  case Value::N_ROWS:
    return model_->getNumCons();
  default:
    throw AMPLSolverException("Specified value unknown.");
  }
}
int CoptCallback::setHeuristicSolution(int nvars, const int* indices, const double* values) {
  std::vector<double> sol(model_->getNumVars());
  for (int i = 0; i < nvars; i++)
    sol[indices[i]] = values[i];
  double obj;
  return COPT_AddCallbackSolution(cbdata_, sol.data(), &obj);
}

std::vector<double> CoptCallback::getValueArray(Value::CBValue v) {
  switch (v)
  {
  case Value::MIP_SOL_RELAXED:
    std::vector<double> d(model_->getNumVars());
    COPT_GetCallbackInfo(cbdata_, COPT_CBINFO_RELAXSOLUTION, d.data());
    return d;
  }
  return std::vector<double>();
}

} // namespace
