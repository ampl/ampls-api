#include "cplex_interface.h"
#include "cplex_callback.h"

const char* CPLEXCallback::getMessage() {
  return msg_;
}

int CPLEXCallback::doAddCut(int nvars, const int* vars,
  const double* coeffs, char direction, double rhs, int lazy) {

  int sense = direction;
  if (lazy)
  { // CPLEX does this by registering two different callbacks. 
    // I can catch it from "where" (see bendersatsp.c example in CPLEX lib)
    if((wherefrom_ == CPX_CALLBACK_MIP_CUT_FEAS) ||  
      (wherefrom_ == CPX_CALLBACK_MIP_CUT_UNBD))
      return CPXcutcallbackadd(env(), NULL, wherefrom_, nvars, rhs, sense, vars,
        coeffs, true);
  }
  else
  {
   if ((wherefrom_ == CPX_CALLBACK_MIP_CUT_LOOP) ||
        (wherefrom_ == CPX_CALLBACK_MIP_CUT_LAST))
    return CPXcutcallbackadd(env(), NULL, wherefrom_, nvars, rhs, sense, vars,
      coeffs, true);
  }
}

int CPLEXCallback::getSolution(int len, double* sol) {

  if((wherefrom_>=CPX_CALLBACK_MIP) && (wherefrom_<= CPX_CALLBACK_MIP_INCUMBENT_MIPSTART))
    return CPXgetcallbackincumbent(env(), this, wherefrom_, sol, 0, len);
}
double CPLEXCallback::getObjective() {
  double obj;
 switch (wherefrom_)
 {
 case CPX_CALLBACK_PRIMAL:
 case CPX_CALLBACK_DUAL:
   int phase;
   CPXgetcallbackinfo(env(), NULL, wherefrom_,
     CPX_CALLBACK_INFO_PRIMAL_FEAS, &phase);
   if (phase != 0) {
     double obj;
     CPXgetcallbackinfo(env(), NULL, wherefrom_,
       CPX_CALLBACK_INFO_PRIMAL_OBJ,
       &obj);
     return obj;
   }
   break;
 case CPX_CALLBACK_MIP_INCUMBENT_NODESOLN:
 case CPX_CALLBACK_MIP_INCUMBENT_HEURSOLN:
 case CPX_CALLBACK_MIP_INCUMBENT_USERSOLN:
 case CPX_CALLBACK_MIP_INCUMBENT_MIPSTART:
   return objval_;
 default:
   return -1;
 }
}

const char* CPLEXCallback::getWhere(int wherefrom)
{
  switch (wherefrom)
  {
  case CPX_CALLBACK_PRIMAL: return "CPX_CALLBACK_PRIMAL";
  case CPX_CALLBACK_DUAL: return "CPX_CALLBACK_DUAL";
  case CPX_CALLBACK_NETWORK: return "CPX_CALLBACK_NETWORK";
  case CPX_CALLBACK_PRIMAL_CROSSOVER: return "CPX_CALLBACK_PRIMAL_CROSSOVER";
  case CPX_CALLBACK_DUAL_CROSSOVER: return "CPX_CALLBACK_DUAL_CROSSOVER";
  case CPX_CALLBACK_BARRIER: return "CPX_CALLBACK_BARRIER";
  case CPX_CALLBACK_PRESOLVE: return "CPX_CALLBACK_PRESOLVE";
  case CPX_CALLBACK_QPBARRIER: return "CPX_CALLBACK_QPBARRIER";
  case CPX_CALLBACK_QPSIMPLEX: return "CPX_CALLBACK_QPSIMPLEX";
  case CPX_CALLBACK_TUNING: return "CPX_CALLBACK_TUNING";
  
    // MIP:
  case CPX_CALLBACK_MIP: return "CPX_CALLBACK_MIP";
  case CPX_CALLBACK_MIP_BRANCH: return "CPX_CALLBACK_MIP_BRANCH";
  case CPX_CALLBACK_MIP_NODE: return "CPX_CALLBACK_MIP_NODE";
  case CPX_CALLBACK_MIP_HEURISTIC: return "CPX_CALLBACK_MIP_HEURISTIC";
  case CPX_CALLBACK_MIP_SOLVE: return "CPX_CALLBACK_MIP_SOLVE";
  case CPX_CALLBACK_MIP_CUT_LOOP: return "CPX_CALLBACK_MIP_CUT_LOOP";
  case CPX_CALLBACK_MIP_PROBE: return "CPX_CALLBACK_MIP_PROBE";
  case CPX_CALLBACK_MIP_FRACCUT: return "CPX_CALLBACK_MIP_FRACCUT";
  case CPX_CALLBACK_MIP_DISJCUT: return "CPX_CALLBACK_MIP_DISJCUT";
  case CPX_CALLBACK_MIP_FLOWMIR: return "CPX_CALLBACK_MIP_FLOWMIR";
  case CPX_CALLBACK_MIP_INCUMBENT_NODESOLN: return "CPX_CALLBACK_MIP_INCUMBENT_NODESOLN";
  case CPX_CALLBACK_MIP_DELETENODE: return "CPX_CALLBACK_MIP_DELETENODE";
  case CPX_CALLBACK_MIP_BRANCH_NOSOLN: return "CPX_CALLBACK_MIP_BRANCH_NOSOLN";
  case CPX_CALLBACK_MIP_CUT_LAST: return "CPX_CALLBACK_MIP_CUT_LAST";
  case CPX_CALLBACK_MIP_CUT_FEAS: return "CPX_CALLBACK_MIP_CUT_FEAS";
  case CPX_CALLBACK_MIP_CUT_UNBD: return "CPX_CALLBACK_MIP_CUT_UNBD";
  case CPX_CALLBACK_MIP_INCUMBENT_HEURSOLN: return "CPX_CALLBACK_MIP_INCUMBENT_HEURSOLN";
  case CPX_CALLBACK_MIP_INCUMBENT_USERSOLN: return "CPX_CALLBACK_MIP_INCUMBENT_USERSOLN";
  case CPX_CALLBACK_MIP_INCUMBENT_MIPSTART: return "CPX_CALLBACK_MIP_INCUMBENT_MIPSTART";
  }
  
  sprintf(CODE, "Unknown where from code: %d", wherefrom);
  return CODE;
}