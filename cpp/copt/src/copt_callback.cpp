#include "copt_callback.h"
#include "copt_interface.h"

namespace ampls
{
const char* CoptCallback::getWhereString()
{
  switch (where_)
  {
  case COPT_CBCONTEXT_MIPRELAX: return "COPT_CBCONTEXT_MIPRELAX";
  case COPT_CBCONTEXT_MIPSOL: return "COPT_CBCONTEXT_MIPSOL";
  default:
    return "Where code not found";
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

int CoptCallback::doAddCut(const ampls::Constraint& c, int lazy) {
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
  /*if ((where_ != GRB_CB_MIPNODE) &&
    (where_ != GRB_CB_MIPSOL))
    throw ampls::AMPLSolverException("The solution vector can be obtained in a callback only from a MIP node or MIP solution callback");
  int flag = where_ == GRB_CB_MIPSOL ? GRB_CB_MIPSOL_SOL :
    GRB_CB_MIPNODE_REL;
  return GRBcbget(cbdata_, where_, flag, sol);*/
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


Variant  CoptCallback::getValue(Value::CBValue v) {
  switch (v)
  {
  case Value::OBJ:
    return Variant(getObj());
  case Value::MIP_RELATIVEGAP:
    return impl::calculateRelMIPGAP(getDouble(COPT_CBINFO_BESTOBJ),
      getDouble(COPT_CBINFO_BESTBND));
  case Value::MIP_OBJBOUND:
    return getDouble(COPT_CBINFO_BESTBND);
  /*case Value::ITERATIONS:
    if (where_ == GRB_CB_SIMPLEX)
      return get(GRB_CB_SPX_ITRCNT);
    if ((where_ >= GRB_CB_MIP) &&
      (where_ >= GRB_CB_MIPNODE))
      return get(GRB_CB_MIP_ITRCNT);
    if (where_ == GRB_CB_BARRIER)
      return get(GRB_CB_BARRIER_ITRCNT);
  case Value::RUNTIME:
    return get(GRB_CB_RUNTIME);*/
  default:
    throw AMPLSolverException("Specified value unknown.");
  }
}
int CoptCallback::setHeuristicSolution(int nvars, const int* indices, const double* values) {
 /* heurUserAction_ = CPX_CALLBACK_SET;
  for (int i = 0; i < nvars; i++)
    x_[indices[i]] = values[i];
  return 0;
*/ // TODO
  return 1;
}

std::vector<double> CoptCallback::getValueArray(Value::CBValue v) {
/*  switch (v)
  {
  case Value::MIP_SOL_RELAXED:
    if (where_ == CPX_CALLBACK_MIP_HEURISTIC)
    {
      std::vector<double> c(x_, x_ + model_->getNumVars());
      return c;
    }
  } TODO
  */
  return std::vector<double>();
}

} // namespace
