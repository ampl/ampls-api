#include "scip_callback.h"
#include "scip_interface.h"

namespace ampls
{


const char* SCIPCallback::getWhereString()
{
  //TODO
  /*
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
  }*/
  return "";
}

Variant SCIPCallback::get(int what)
{
  /*
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
  */
  return Variant(0);
}

void SCIPCallback::terminate() {
  //GRBterminate(getGRBModel());
  //TODO
}
SCIP* SCIPCallback::getSCIPModel() {
    return ((SCIP*)model_);
  };
const char* SCIPCallback::getMessage()
{
  return msg_.data();
}

int SCIPCallback::doAddCut(const ampls::Constraint& c, int lazy) {
  char sense = toSCIPSense(c.sense());
    
  //OsiCuts_addRowCut(osicuts_, c.coeffs().size(), c.indices().data(),
  //      c.coeffs().data(), sense, c.rhs());
  return 0;
}

int SCIPCallback::getSolution(int len, double* sol)
{ 
  //int ncc = Osi_getNumCols(osisolver_);
  //auto osiSol = Osi_getColSolution(osisolver_);
  //for (int i = 0; i < len; i++)
  //  sol[i] = osiSol[i];
  return 0;
}

double SCIPCallback::getObj()
{
  //return Cbc_getObjValue(getCBCModel()->getCBCmodel());
  return 0.0;
}


Variant SCIPCallback::getValue(Value::CBValue v) {
  
  
  switch (v)
  {
  case Value::OBJ:
    return Variant(getObj());
  case Value::MIP_RELATIVEGAP:
    return impl::calculateRelMIPGAP(getObj(),
      getValue(Value::MIP_OBJBOUND).dbl);
  case Value::MIP_OBJBOUND:
    //return Cbc_getBestPossibleObjValue(getCBCModel()->getCBCmodel());
  default:
    throw AMPLSolverException("Specified value unknown.");
  }
  return Variant(0);
}

int SCIPCallback::setHeuristicSolution(int nvars, const int* indices, const double* values) {
  /*
  std::vector<double> vals(model_->getNumVars(), GRB_UNDEFINED);
  for (int i = 0; i < nvars; i++)
    vals[indices[i]] = values[i]; 
    double objective;
  return GRBcbsolution(cbdata_, vals.data(), &objective);
  */
  return 0;
}


} // namespace

