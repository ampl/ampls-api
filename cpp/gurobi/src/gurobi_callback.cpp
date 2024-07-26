#include "gurobi_callback.h"
#include "gurobi_interface.h"

namespace ampls
{
const char* GurobiCallback::getWhereString()
{
  switch (where_)
  {
  case GRB_CB_POLLING: return "GRB_CB_POLLING";
  case GRB_CB_PRESOLVE: return "GRB_CB_PRESOLVE";
  case GRB_CB_SIMPLEX: return "GRB_CB_SIMPLEX";
  case GRB_CB_MIP: return "GRB_CB_MIP";
  case GRB_CB_MIPSOL: return "GRB_CB_MIPSOL";
  case GRB_CB_MIPNODE: return "GRB_CB_MIPNODE";
  case GRB_CB_MESSAGE: return "GRB_CB_MESSAGE";
  case GRB_CB_BARRIER: return "GRB_CB_BARRIER";
  case GRB_CB_MULTIOBJ: return "GRB_CB_MULTIOBJ";
  default:
    return "Where code not found";
  }
}

Variant GurobiCallback::get(int what)
{
  Variant r = Variant();
  switch (what)
  {
    // Presolve int
  case GRB_CB_PRE_COLDEL:
  case GRB_CB_PRE_ROWDEL:
  case GRB_CB_PRE_SENCHG:
  case GRB_CB_PRE_BNDCHG:
  case GRB_CB_PRE_COECHG:
    // Simplex int
  case GRB_CB_SPX_ISPERT:
    // MIP int
  case GRB_CB_MIP_SOLCNT:
  case GRB_CB_MIP_CUTCNT:
    // MIPSSOL int
  case GRB_CB_MIPSOL_SOLCNT:
    // MIPNODE int
  case GRB_CB_MIPNODE_SOLCNT:
  case GRB_CB_MIPNODE_STATUS:
    // Barrier int
  case GRB_CB_BARRIER_ITRCNT:
    // Multiobj int
  case GRB_CB_MULTIOBJ_OBJCNT:
  case GRB_CB_MULTIOBJ_SOLCNT:
    r.type = 1;
    r.integer = getInt(what);
    break;
    // Generic double  
  case GRB_CB_RUNTIME:
    // Simplex double
  case GRB_CB_SPX_ITRCNT:
  case GRB_CB_SPX_OBJVAL:
  case GRB_CB_SPX_PRIMINF:
  case GRB_CB_SPX_DUALINF:
    // MIP double
  case GRB_CB_MIP_OBJBST:
  case GRB_CB_MIP_OBJBND:
  case GRB_CB_MIP_NODCNT:
  case GRB_CB_MIP_NODLFT:
  case GRB_CB_MIP_ITRCNT:
 // case GRB_CB_MIP_OBJBNDC:
    // MIPSol double
  case GRB_CB_MIPSOL_OBJ:
  case GRB_CB_MIPSOL_OBJBST:
  case GRB_CB_MIPSOL_OBJBND:
  case GRB_CB_MIPSOL_NODCNT:
 // case GRB_CB_MIPSOL_OBJBNDC:
    // Mipnode double
  case GRB_CB_MIPNODE_OBJBST:
  case GRB_CB_MIPNODE_OBJBND:
  case GRB_CB_MIPNODE_NODCNT:
    // Barrier double
  case GRB_CB_BARRIER_PRIMOBJ:
  case GRB_CB_BARRIER_DUALOBJ:
  case GRB_CB_BARRIER_PRIMINF:
  case GRB_CB_BARRIER_DUALINF:
  case GRB_CB_BARRIER_COMPL:
    r.type = 2;
    r.dbl = getDouble(what);
    break;
  case GRB_CB_MSG_STRING:
    r.type = 0;
    r.str = getMessage();
  }
  return r;
  // arrays
  //case GRB_CB_MIPSOL_SOL     4001
  //case GRB_CB_MIPNODE_REL     5002
  // case GRB_CB_MULTIOBJ_SOL     8003

  // Not documented
  //  case GRB_CB_MIPNODE_BRVAR   5007
  // case GRB_CB_MIPNODE_OBJBNDC 5008
}

void GurobiCallback::terminate() {
  GRBterminate(getGRBModel());
}
   GRBmodel* GurobiCallback::getGRBModel() {
    return ((GurobiModel*)model_)->getGRBmodel();
  };
const char* GurobiCallback::getMessage()
{
  char* msg;
  GRBcbget(cbdata_, where_, GRB_CB_MSG_STRING, &msg);
  return msg;
}

int GurobiCallback::doAddCut(const ampls::Constraint& c, int lazy, void* additionalParams) {
  (void)additionalParams;
  char sense = toGRBSense(c.sense());
  if (lazy)
  {
    return GRBcblazy(cbdata_, static_cast<int>(c.indices().size()), 
      c.indices().data(), c.coeffs().data(), sense, c.rhs());
  }
  else
  {
    return GRBcbcut(cbdata_, static_cast<int>(c.indices().size()), c.indices().data(),
      c.coeffs().data(), sense, c.rhs());
  }
}

int GurobiCallback::getSolution(int len, double* sol)
{
  if ((where_ != GRB_CB_MIPNODE) &&
    (where_ != GRB_CB_MIPSOL))
    throw ampls::AMPLSolverException("The solution vector can be obtained in a callback only from a MIP node or MIP solution callback");
  int flag = where_ == GRB_CB_MIPSOL ? GRB_CB_MIPSOL_SOL :
    GRB_CB_MIPNODE_REL;
  return GRBcbget(cbdata_, where_, flag, sol);
}
double GurobiCallback::getObjBnd()
{
  int flag;
  switch (where_) {
  case GRB_CB_MIP:
    flag = GRB_CB_MIP_OBJBND;
    break;
  case GRB_CB_MIPSOL:
    flag = GRB_CB_MIPSOL_OBJBND;
    break;
  case GRB_CB_MIPNODE:
    flag = GRB_CB_MIPNODE_OBJBND;
    break;
  default:
    throw ampls::AMPLSolverException("Cannot get objective bound from here!");
  }
  return getDouble(flag);

}
double GurobiCallback::getObj()
{
  int flag;
  switch (where_)
  {
  case GRB_CB_SIMPLEX:
    flag = GRB_CB_SPX_OBJVAL;
    break;
  case GRB_CB_MIP:
    flag = GRB_CB_MIP_OBJBST;
    break;
  case GRB_CB_MIPSOL:
    flag = GRB_CB_MIPSOL_OBJ;
    break;
  case GRB_CB_MIPNODE:
    flag = GRB_CB_MIPNODE_OBJBST;
    break;
  case GRB_CB_BARRIER:
    flag = GRB_CB_BARRIER_PRIMOBJ;
    break;
  default:
    throw ampls::AMPLSolverException("Cannot get objective value from here!");
  }
  return getDouble(flag);
}


Variant  GurobiCallback::getValueImpl(Value::CBValue v) {
  int f = 0;
  switch (v)
  {
  case Value::OBJ:
    return Variant(getObj());
  case Value::MIP_RELATIVEGAP:
    return Variant(impl::calculateRelMIPGAP(getObj(), getObjBnd()));
  case Value::MIP_OBJBOUND:
    return Variant(getObjBnd());
  case Value::PRE_DELCOLS:
    return get(GRB_CB_PRE_COLDEL);
  case Value::PRE_DELROWS:
    return get(GRB_CB_PRE_ROWDEL);
  case Value::PRE_COEFFCHANGED:
    return get(GRB_CB_PRE_COECHG);
  case Value::ITERATIONS:
    if (where_ == GRB_CB_SIMPLEX)
      return get(GRB_CB_SPX_ITRCNT);
    if ((where_ >= GRB_CB_MIP) &&
      (where_ <= GRB_CB_MIPNODE))
      return get(GRB_CB_MIP_ITRCNT);
    if (where_ == GRB_CB_BARRIER)
      return get(GRB_CB_BARRIER_ITRCNT);
  case Value::RUNTIME:
    return get(GRB_CB_RUNTIME);
  case Value::MIP_NODES:
    
    if (where_ == GRB_CB_MIP) f = GRB_CB_MIP_NODCNT;
    else if (where_ == GRB_CB_MIPSOL) f = GRB_CB_MIPSOL_NODCNT;
    else if (where_ == GRB_CB_MIPNODE) f = GRB_CB_MIPNODE_NODCNT;
    else throw ampls::AMPLSolverException("Cannot get this value from here.");
    return Variant(int(getDouble(f)));
  case Value::N_COLS:
    return model_->getNumVars();
  case Value::N_ROWS:
    return model_->getNumCons();
  default:
    throw AMPLSolverException("Specified value unknown.");
  }
}

int GurobiCallback::setHeuristicSolution(int nvars, const int* indices, const double* values) {
  std::vector<double> vals(model_->getNumVars(), GRB_UNDEFINED);
  for (int i = 0; i < nvars; i++)
    vals[indices[i]] = values[i]; 
    double objective;
  return GRBcbsolution(cbdata_, vals.data(), &objective);
}


} // namespace
