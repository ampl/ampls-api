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
  case GRB_CB_MIP_OBJBNDC:
    // MIPSol double
  case GRB_CB_MIPSOL_OBJ:
  case GRB_CB_MIPSOL_OBJBST:
  case GRB_CB_MIPSOL_OBJBND:
  case GRB_CB_MIPSOL_NODCNT:
  case GRB_CB_MIPSOL_OBJBNDC:
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

int GurobiCallback::doAddCut(int nvars, const int* vars,
  const double* coeffs, CutDirection::Direction direction, double rhs, int lazy) {
  char sense;
  switch (direction)
  {
  case CutDirection::EQ:
    sense = GRB_EQUAL;
    break;
  case CutDirection::GE:
    sense = GRB_GREATER_EQUAL;
    break;
  case CutDirection::LE:
    sense = GRB_LESS_EQUAL;
    break;
  default:
    throw AMPLSolverException("Unexpected cut direction");
  }
  if (lazy)
  {
    return GRBcblazy(cbdata_, nvars, vars,
      coeffs, sense, rhs);
  }
  else
  {
    return GRBcbcut(cbdata_, nvars, vars,
      coeffs, sense, rhs);
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
    flag = GRB_CB_MIPSOL_OBJBST;
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
  double obj;
  GRBcbget(cbdata_, where_, flag, &obj);
  return obj;
}
} // namespace
