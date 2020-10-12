#include "cplex_interface.h"
#include "cplex_callback.h"

namespace ampls
{
char errbuf[CPXMESSAGEBUFSIZE];
const char* CPLEXCallback::getMessage() {
  return msg_;
}

int CPLEXCallback::doAddCut(int nvars, const int* vars,
  const double* coeffs, CutDirection::Direction direction, double rhs, int lazy) {
  char sense;
  switch (direction)
  {
    case CutDirection::EQ:
      sense = 'E';
      break;
    case CutDirection::GE:
      sense = 'G';
      break;
    case CutDirection::LE:
      sense = 'L';
      break;
    default:
      throw AMPLSolverException("Unexpected cut direction");
  }

  int res;
  if (lazy)
  { // CPLEX does this by registering two different callbacks. 
    // I can catch it from "where" (see bendersatsp.c example in CPLEX lib)
    if ((where_ == CPX_CALLBACK_MIP_CUT_FEAS) ||
      (where_ == CPX_CALLBACK_MIP_CUT_UNBD)
      )
    {
      res = CPXcutcallbackadd(getCPXENV(), cbdata_, where_, nvars, rhs, sense, vars,
        coeffs, CPX_USECUT_FORCE);
    }
    else
      return 0;
  }
  else
  {
    if ((where_ == CPX_CALLBACK_MIP_CUT_LOOP) ||
      (where_ == CPX_CALLBACK_MIP_CUT_LAST))
      res = CPXcutcallbackadd(getCPXENV(), cbdata_, where_, nvars, rhs, sense, vars,
        coeffs, CPX_USECUT_FILTER);
    else
      return 0;
  }
  if (res != 0) {
    fprintf(stderr, "Failed to add %s: %s\n", lazy ? "lazy constraint" : "user cut",
      CPXgeterrorstring(getCPXENV(), res, errbuf));
  }
  return res;
}

int CPLEXCallback::getSolution(int len, double* sol) {
  if ((where_ == CPX_CALLBACK_MIP_CUT_FEAS) || (where_ == CPX_CALLBACK_MIP_CUT_UNBD) ||
    (where_== CPX_CALLBACK_MIP_CUT_LOOP) || (where_ == CPX_CALLBACK_MIP_CUT_LAST))
  {
    int error = CPXgetcallbacknodex(getCPXENV(), cbdata_, where_, sol, 0, len - 1);
    if (error != 0)
    fprintf(stderr, "Failed to retrieve solution nodex from callback: %s\n",
      CPXgeterrorstring(getCPXENV(), error, errbuf));
    return 0;
  }
  if ((where_ >= CPX_CALLBACK_MIP) && (where_ <= CPX_CALLBACK_MIP_INCUMBENT_MIPSTART))
  {
    int error = CPXgetcallbackincumbent(getCPXENV(), cbdata_, where_, sol, 0, len-1);
    if (error!= 0)
        fprintf(stderr, "Failed to retrieve solution from callback: %s\n",
          CPXgeterrorstring(getCPXENV(), error, errbuf));
    return 0;
  }
  throw ampls::AMPLSolverException("Cannot get the solution vector in this stage.");
}
double CPLEXCallback::getObj() {
  int phase = -1;
  switch (where_)
  {
  case CPX_CALLBACK_PRIMAL:
  case CPX_CALLBACK_DUAL:
    phase = getInt(CPX_CALLBACK_INFO_PRIMAL_FEAS);
    if (phase != 0)
      return getDouble(CPX_CALLBACK_INFO_PRIMAL_OBJ);
    break;
  case CPX_CALLBACK_MIP_INCUMBENT_NODESOLN:
  case CPX_CALLBACK_MIP_INCUMBENT_HEURSOLN:
  case CPX_CALLBACK_MIP_INCUMBENT_USERSOLN:
  case CPX_CALLBACK_MIP_INCUMBENT_MIPSTART:
    return objval_;
  default:
    throw ampls::AMPLSolverException("Cannot get the objective value in this stage.");
  }
  throw ampls::AMPLSolverException::format("Cannot get the objective value in this stage (%s)", getWhereString());
}
const char* CPLEXCallback::getWhereString()
{
  switch (where_)
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

  sprintf(CODE, "Unknown where from code: %d", where_);
  return CODE;
}

Variant CPLEXCallback::get(int what)
{
  Variant r = Variant();
  switch (what)
  {
  case CPX_CALLBACK_INFO_PRIMAL_FEAS:
  case CPX_CALLBACK_INFO_DUAL_FEAS:
  case CPX_CALLBACK_INFO_ITCOUNT:
  case CPX_CALLBACK_INFO_CROSSOVER_PPUSH:
  case CPX_CALLBACK_INFO_CROSSOVER_PEXCH:
  case CPX_CALLBACK_INFO_CROSSOVER_DPUSH:
  case CPX_CALLBACK_INFO_CROSSOVER_DEXCH:
  case CPX_CALLBACK_INFO_PRESOLVE_ROWSGONE:
  case CPX_CALLBACK_INFO_PRESOLVE_COLSGONE:
  case CPX_CALLBACK_INFO_PRESOLVE_COEFFS:
  case CPX_CALLBACK_INFO_PRESOLVE_AGGSUBST:
  case CPX_CALLBACK_INFO_CROSSOVER_SBCNT:
    // MIP
  case CPX_CALLBACK_INFO_NODE_COUNT:
  case CPX_CALLBACK_INFO_NODES_LEFT:
  case CPX_CALLBACK_INFO_MIP_ITERATIONS:
  case CPX_CALLBACK_INFO_MIP_FEAS:
  case CPX_CALLBACK_INFO_CLIQUE_COUNT:
  case CPX_CALLBACK_INFO_COVER_COUNT:
  case CPX_CALLBACK_INFO_FLOWCOVER_COUNT:
  case CPX_CALLBACK_INFO_GUBCOVER_COUNT:
  case CPX_CALLBACK_INFO_IMPLBD_COUNT:
  case CPX_CALLBACK_INFO_PROBE_PHASE:
  case CPX_CALLBACK_INFO_FRACCUT_COUNT:
  case CPX_CALLBACK_INFO_DISJCUT_COUNT:
  case CPX_CALLBACK_INFO_FLOWPATH_COUNT:
  case CPX_CALLBACK_INFO_MIRCUT_COUNT:
  case CPX_CALLBACK_INFO_ZEROHALFCUT_COUNT:
  case CPX_CALLBACK_INFO_MY_THREAD_NUM:
  case CPX_CALLBACK_INFO_USER_THREADS:
  case CPX_CALLBACK_INFO_MCFCUT_COUNT:
  case CPX_CALLBACK_INFO_LANDPCUT_COUNT:
  case CPX_CALLBACK_INFO_USERCUT_COUNT:
  case CPX_CALLBACK_INFO_TABLECUT_COUNT:
  case CPX_CALLBACK_INFO_SOLNPOOLCUT_COUNT:
  case CPX_CALLBACK_INFO_BENDERS_COUNT:
  case CPX_CALLBACK_INFO_NODE_COUNT_LONG:
  case CPX_CALLBACK_INFO_NODES_LEFT_LONG:
  case CPX_CALLBACK_INFO_MIP_ITERATIONS_LONG:

    // NODE
  case CPX_CALLBACK_INFO_NODE_NIINF:
  case CPX_CALLBACK_INFO_NODE_DEPTH:
  case CPX_CALLBACK_INFO_NODE_TYPE: // Note that this is char
  case CPX_CALLBACK_INFO_NODE_VAR:
  case CPX_CALLBACK_INFO_NODE_SOS:
  case CPX_CALLBACK_INFO_NODE_SEQNUM:
  case CPX_CALLBACK_INFO_NODE_NODENUM:
  case CPX_CALLBACK_INFO_NODE_SEQNUM_LONG:
  case CPX_CALLBACK_INFO_NODE_NODENUM_LONG:
  case CPX_CALLBACK_INFO_NODE_DEPTH_LONG:

    // SOS
  case CPX_CALLBACK_INFO_SOS_TYPE: // This is char
  case CPX_CALLBACK_INFO_SOS_NUM:
  case CPX_CALLBACK_INFO_SOS_SIZE:
  case CPX_CALLBACK_INFO_SOS_IS_FEASIBLE:
  case CPX_CALLBACK_INFO_SOS_MEMBER_INDEX:

    /* Values for getcallbackindicatorinfo function */
  case CPX_CALLBACK_INFO_IC_NUM:
  case CPX_CALLBACK_INFO_IC_IMPLYING_VAR:
  case CPX_CALLBACK_INFO_IC_IMPLIED_VAR:
  case CPX_CALLBACK_INFO_IC_SENSE:
  case CPX_CALLBACK_INFO_IC_COMPL:
  case CPX_CALLBACK_INFO_IC_IS_FEASIBLE:

    r.type = 1;
    r.integer = getInt(what);
    return r;

  case CPX_CALLBACK_INFO_PRIMAL_OBJ:
  case CPX_CALLBACK_INFO_DUAL_OBJ:
  case CPX_CALLBACK_INFO_PRIMAL_INFMEAS:
  case CPX_CALLBACK_INFO_DUAL_INFMEAS:
  case CPX_CALLBACK_INFO_ENDTIME:
  case CPX_CALLBACK_INFO_TUNING_PROGRESS:



    // MIP
  case CPX_CALLBACK_INFO_BEST_INTEGER:
  case CPX_CALLBACK_INFO_BEST_REMAINING:
  case CPX_CALLBACK_INFO_CUTOFF:
  case CPX_CALLBACK_INFO_PROBE_PROGRESS:
  case CPX_CALLBACK_INFO_FRACCUT_PROGRESS:
  case CPX_CALLBACK_INFO_DISJCUT_PROGRESS:
  case CPX_CALLBACK_INFO_FLOWMIR_PROGRESS:
  case CPX_CALLBACK_INFO_MIP_REL_GAP:
  case CPX_CALLBACK_INFO_KAPPA_STABLE:
  case CPX_CALLBACK_INFO_KAPPA_SUSPICIOUS:
  case CPX_CALLBACK_INFO_KAPPA_UNSTABLE:
  case CPX_CALLBACK_INFO_KAPPA_ILLPOSED:
  case CPX_CALLBACK_INFO_KAPPA_MAX:
  case CPX_CALLBACK_INFO_KAPPA_ATTENTION:

    // NODE
  case CPX_CALLBACK_INFO_NODE_SIINF:
  case CPX_CALLBACK_INFO_NODE_ESTIMATE:
  case CPX_CALLBACK_INFO_NODE_OBJVAL:

    // SOS
  case CPX_CALLBACK_INFO_SOS_MEMBER_REFVAL:

    // Indicator
  case CPX_CALLBACK_INFO_IC_RHS:
    r.type = 2;
    r.dbl = getDouble(what);
    break;
  }
  return r;
}

} // namespace