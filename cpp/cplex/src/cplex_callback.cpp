#include "cplex_interface.h"
#include "cplex_callback.h"

#define CPLEX_CALL( call ) do { if (int e=call) \
  throw AMPLSolverException::format( \
    "  Call failed: %s with code %d", #call, e ); } while (0);


namespace ampls
{
char errbuf[CPXMESSAGEBUFSIZE];

int CPLEXCallback::getMaxThreads() {
  int num_cores;
  auto env = ((ampls::CPLEXModel*)model_)->getCPXENV();

  int threads = model_->getIntOption("threads");
  if (threads == 0)
  {
    CPXgetnumcores(env, &num_cores);
    return num_cores;
  }
  return threads;
}


bool CPLEXCallback::canDo(CanDo::Functionality f, int threadid) {
  switch (f) {
    case CanDo::GET_MIP_SOLUTION:
    case CanDo::ADD_LAZY_CONSTRAINT:
      if (getWhere(threadid) != CPX_CALLBACKCONTEXT_CANDIDATE) return false;
      int point;
      CPXcallbackcandidateispoint(context(threadid), &point);
      return point!=0;
    case CanDo::GET_LP_SOLUTION:
    case CanDo::ADD_USER_CUT:
      return getWhere(threadid) == CPX_CALLBACKCONTEXT_RELAXATION;
    case CanDo::IMPORT_SOLUTION:
      return (getWhere(threadid) != CPX_CALLBACKCONTEXT_THREAD_UP) &&
        (getWhere(threadid) != CPX_CALLBACKCONTEXT_THREAD_DOWN);
    default:
      return false;
  }


}

double CPLEXCallback::getCPLEXDouble(CPXCALLBACKINFO what, int threadid)
{
  double res;
  CPLEX_CALL(CPXcallbackgetinfodbl(context(threadid), what, &res));
  return res;
}

int CPLEXCallback::getCPLEXInt(CPXCALLBACKINFO what, int threadid)
{
  int res;
  CPLEX_CALL(CPXcallbackgetinfoint(context(threadid), what, &res));
  return res;
}

long CPLEXCallback::getCPLEXLong(CPXCALLBACKINFO what, int threadid)
{
  CPXLONG res;
  CPLEX_CALL(CPXcallbackgetinfolong(context(threadid), what, &res));
  return res;
}

Variant CPLEXCallback::getValueImpl(Value::CBValue v) {
  switch (v)
  {
    case Value::ITERATIONS:
      return getCPLEXInfo(CPXCALLBACKINFO_ITCOUNT);
    case Value::MIP_NODES:
      return getCPLEXInfo(CPXCALLBACKINFO_NODECOUNT);
    case Value::RUNTIME:
      return getCPLEXInfo(CPXCALLBACKINFO_TIME);
    case Value::MIP_OBJBOUND:
      return getCPLEXInfo(CPXCALLBACKINFO_BEST_BND); 
    case Value::OBJ:
      return Variant(getObj());
    case Value::MIP_RELATIVEGAP:
    return Variant(impl::calculateRelMIPGAP(getObj(),
      getCPLEXDouble(CPXCALLBACKINFO_BEST_BND)));
    case Value::N_COLS:
      return model_->getNumVars();
    case Value::N_ROWS:
      return model_->getNumCons();
  default: throw AMPLSolverException("Specified value unknown or unsupported");
  }
}

Variant CPLEXCallback::getValueImpl(Value::CBValue v, int threadid)
 {
  switch (v)
  {
  case Value::ITERATIONS:
    return getCPLEXInfo(CPXCALLBACKINFO_ITCOUNT, threadid);
  case Value::MIP_NODES:
    return getCPLEXInfo(CPXCALLBACKINFO_NODECOUNT, threadid);
  case Value::RUNTIME:
    return getCPLEXInfo(CPXCALLBACKINFO_TIME, threadid);
  case Value::MIP_OBJBOUND:
    return getCPLEXInfo(CPXCALLBACKINFO_BEST_BND, threadid);
  case Value::OBJ:
    return Variant(getObj());
  case Value::MIP_RELATIVEGAP:
    return Variant(impl::calculateRelMIPGAP(getCPLEXDouble(CPXCALLBACKINFO_BEST_SOL, threadid),
      getCPLEXDouble(CPXCALLBACKINFO_BEST_BND, threadid)));
  case Value::N_COLS:
    return model_->getNumVars();
  case Value::N_ROWS:
    return model_->getNumCons();
  default: throw AMPLSolverException("Specified value unknown or unsupported");
  }
}


int CPLEXCallback::doAddCut(const ampls::Constraint& c, int lazy, void* additionalParams) {
  double rhs[1] = { c.rhs() };
  char sense[1] = { toCPLEXSense(c.sense()) };
  int res;

  int purgeable = additionalParams ? static_cast<CPLEX_CB_PARAMS*>(additionalParams)->purgeable : CPX_USECUT_FORCE;
  int local = additionalParams ? static_cast<CPLEX_CB_PARAMS*>(additionalParams)->local : 0;
  int beg = 0;
  if (lazy)
  { 
    if (!canDo(CanDo::ADD_LAZY_CONSTRAINT))
      throw ampls::AMPLSolverException("Functionality not available at this stage");
    if(local)
      res = CPXcallbackrejectcandidatelocal(context(0), 1, c.indices().size(), rhs, sense, &beg, c.indices().data(),
        c.coeffs().data());
    else
    res= CPXcallbackrejectcandidate(context(0), 1, c.indices().size(), rhs, sense, &beg, c.indices().data(),
      c.coeffs().data());
  }
  else
  {
    if (!canDo(CanDo::ADD_USER_CUT))
      throw ampls::AMPLSolverException("Functionality not available at this stage");
    res = CPXcallbackaddusercuts(context(0), 1, c.indices().size(), rhs, sense, &beg, c.indices().data(),
      c.coeffs().data(), &purgeable, &local);
  }
  if (res != 0) {
    fprintf(stderr, "Failed to add %s: %s\n", lazy ? "lazy constraint" : "user cut",
      CPXgeterrorstring(((CPLEXModel*)model_)->getCPXENV(), res, errbuf));
  }
  return res;
}

int CPLEXCallback::getSolution(int len, double* sol) {
  double obj;
  if (getAMPLWhereImpl() == Where::MIPSOL) {
    return CPXXcallbackgetcandidatepoint(context(0), sol, 0, len - 1, &obj);
  }
  else if (getAMPLWhereImpl() == Where::MIPNODE) {
    return CPXXcallbackgetrelaxationpoint(context(0), sol, 0, len - 1, &obj);
  }
  throw ampls::AMPLSolverException("Cannot get the solution vector in this stage.");
}
double CPLEXCallback::getObj() {
  // TODO Check for global vs thread-local objective
  return getCPLEXDouble(CPXCALLBACKINFO_BEST_SOL);
}

#define _CASE(EnumValue) case EnumValue: return #EnumValue

const char* CPLEXCallback::getWhereString(int threadid)
{
  switch (getWhere(threadid))
  {
    _CASE(CPX_CALLBACKCONTEXT_BRANCHING);
    _CASE(CPX_CALLBACKCONTEXT_CANDIDATE);
    _CASE(CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS);
    _CASE(CPX_CALLBACKCONTEXT_LOCAL_PROGRESS);
    _CASE(CPX_CALLBACKCONTEXT_RELAXATION);
    _CASE(CPX_CALLBACKCONTEXT_THREAD_DOWN);
    _CASE(CPX_CALLBACKCONTEXT_THREAD_UP);
  }
  sprintf(CODE, "Unknown where from code: %d", where_);
  return CODE;
}

Variant CPLEXCallback::getCPLEXInfo(CPXCALLBACKINFO what, int threadid)
{
  Variant r;
  switch(what) {
  case CPXCALLBACKINFO_ITCOUNT:
  case CPXCALLBACKINFO_THREADID:
  case CPXCALLBACKINFO_NODECOUNT:
  case CPXCALLBACKINFO_FEASIBLE:
  case CPXCALLBACKINFO_THREADS:
  case CPXCALLBACKINFO_CANDIDATE_SOURCE:
  case CPXCALLBACKINFO_RESTARTS:
  case CPXCALLBACKINFO_AFTERCUTLOOP:
    r.type = 1;
    r.integer = getCPLEXInt(what, threadid);
    break;

  // long
  case CPXCALLBACKINFO_NODEUID:
  case CPXCALLBACKINFO_NODEDEPTH:
  case CPXCALLBACKINFO_NODESLEFT:
    r.type = 1;
    r.integer = getCPLEXLong(what, threadid);
    break;

  case CPXCALLBACKINFO_BEST_SOL:
  case CPXCALLBACKINFO_BEST_BND:
  case CPXCALLBACKINFO_TIME:
  case CPXCALLBACKINFO_DETTIME:
    r.type = 2;
    r.dbl = getCPLEXDouble(what, threadid);
    break;
  }
  return r;
}

const char toCPLEXMap[3] = { 'E', 'G', 'L' };

char CPLEXCallback::toCPLEXSense(ampls::CutDirection::Direction direction)
{
  return toCPLEXMap[(int)direction];
}

int CPLEXCallback::setHeuristicSolution(int nvars, const int* indices, const double* values) {
  // TODO what to do for obj value?
  return CPXcallbackpostheursoln(context(0), nvars, indices, values, INFINITY,
    CPXCALLBACKSOLUTION_SOLVE);
}

std::vector<double> CPLEXCallback::getValueArray(Value::CBValue v) {
  switch (v)
  {
  case Value::MIP_SOL_RELAXED:
    if (canDo(CanDo::GET_LP_SOLUTION)) {
      int NVARS = model_->getNumVars();
      std::vector<double> res(NVARS);
      CPXcallbackgetrelaxationpoint(context(0), res.data(), 0, NVARS - 1, NULL);
      return res;
    }
  }
  return std::vector<double>();
}

} // namespace