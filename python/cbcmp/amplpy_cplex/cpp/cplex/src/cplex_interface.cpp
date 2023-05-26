#include "cplex_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr

namespace ampls
{
CPLEXCallback* impl::cpx::CBWrap::setDefaultCB(CPXCENVptr env, void* cbdata,
  int wherefrom, void* userhandle, int capabilities)
{
  CPLEXCallback* cb = static_cast<CPLEXCallback*>(userhandle);
  cb->where_ = wherefrom;
  cb->env_ = env;
  cb->cbdata_ = cbdata;
  cb->currentCapabilities_ = capabilities;
  return cb;
}

int CPXPUBLIC impl::cpx::CBWrap::incumbent_callback_wrapper(CPXCENVptr env, void* cbdata,
  int wherefrom, void* userhandle,
  double objval, double* x, int* isfeas_p, int* useraction_p) {

  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle, 0);

  *isfeas_p = 1; // use new solution by default
  cb->objval_ = objval;
  cb->x_ = x;
  if (cb->run())
    *useraction_p = CPX_CALLBACK_FAIL;
  else
    *useraction_p = CPX_CALLBACK_DEFAULT;
  return 0;
}

int CPXPUBLIC impl::cpx::CBWrap::heuristiccallbackfunc_wrapper(CPXCENVptr env,
  void* cbdata, int wherefrom, void* userhandle, 
  double* objval_p, double* x, int* checkfeas_p, int* useraction_p) {
  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle, ampls::CanDo::IMPORT_SOLUTION |
  ampls::CanDo::GET_LP_SOLUTION);
  cb->objval_ = *objval_p;
  cb->x_ = x;
  cb->heurUserAction_ = CPX_CALLBACK_DEFAULT;
  cb->run();
  // This is set by the setHeuristic solution function
  if (cb->heurUserAction_ == CPX_CALLBACK_SET)
  {
    *objval_p = cb->objval_;
    *checkfeas_p = cb->heurCheckFeas_;
    *useraction_p = cb->heurUserAction_;
  }
  else
    *useraction_p = CPX_CALLBACK_DEFAULT;
  return 0;
}

int CPXPUBLIC impl::cpx::CBWrap::lp_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
    void* userhandle)
{
  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle, 0);
  return cb->run();
}

int CPXPUBLIC  impl::cpx::CBWrap::cut_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
  void* userhandle, int* useraction_p)
{
  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle, 0);
  int res = cb->run();
  if (res)
    *useraction_p = CPX_CALLBACK_FAIL;
  else
    *useraction_p = CPX_CALLBACK_DEFAULT;
  return 0;
}

bool impl::cpx::CBWrap::skipMsgCallback = false;
void CPXPUBLIC
impl::cpx::CBWrap::msg_callback_wrapper(void* handle, const char* msg)
{

  if (impl::cpx::CBWrap::skipMsgCallback)
    return;

  CPLEXCallback* cb = static_cast<CPLEXCallback*>(handle);
  cb->where_ = -1;
  cb->currentCapabilities_ = 0;
  cb->msg_ = msg;
  cb->run();
}

CPLEXDrv::~CPLEXDrv() {
}

void CPLEXDrv::freeCPLEXEnv()
{
  //CPXENVptr env = getEnv();
  //CPXcloseCPLEX(&env);
}

CPLEXModel CPLEXDrv::loadModelImpl(char** args) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::cpx::AMPLSOpen_cplexmp(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return CPLEXModel(mp, args[1]);
}
CPLEXModel CPLEXDrv::loadModel(const char* modelName) {
  return loadModelGeneric(modelName);
}
int setMsgCallback(impl::BaseCallback* callback, CPXENVptr env) {
  impl::cpx::CBWrap::skipMsgCallback = false;
  /* Now get the standard channels.  If an error, just call our
      message function directly. */
  CPXCHANNELptr cpxresults, cpxwarning, cpxerror, cpxlog;
  char errmsg[CPXMESSAGEBUFSIZE];
  int status = CPXgetchannels(env, &cpxresults, &cpxwarning, &cpxerror, &cpxlog);
  if (status) {
    fprintf(stderr, "Could not get standard channels.\n");
    CPXgeterrorstring(env, status, errmsg);
    fprintf(stderr, "%s\n", errmsg);
    return -1;
  }

  /* Now set up the error channel first.  The label will be "cpxerror" */

  status = CPXaddfuncdest(env, cpxerror, callback, impl::cpx::CBWrap::msg_callback_wrapper);
  if (status) {
    fprintf(stderr, "Could not set up error message handler.\n");
    CPXgeterrorstring(env, status, errmsg);
    fprintf(stderr, "%s\n", errmsg);
  }

  /* Now that we have the error message handler set up, all CPLEX
      generated errors will go through ourmsgfunc.  So we don't have
      to use CPXgeterrorstring to determine the text of the message. */

  status = CPXaddfuncdest(env, cpxwarning, callback, impl::cpx::CBWrap::msg_callback_wrapper);
  if (status) {
    impl::cpx::CBWrap::msg_callback_wrapper(callback, "Failed to set up handler for cpxwarning.\n");
    return 1;
  }

  status = CPXaddfuncdest(env, cpxresults, callback, impl::cpx::CBWrap::msg_callback_wrapper);
  if (status) {
    impl::cpx::CBWrap::msg_callback_wrapper(callback, "Failed to set up handler for cpxresults.\n");
    return 1;
  }
  return 0;
}

int CPLEXModel::setCallbackDerived(impl::BaseCallback* callback) {
  CPXENVptr p = getCPXENV();
  // Add the callback 
  int status = CPXsetlazyconstraintcallbackfunc(p, impl::cpx::CBWrap::cut_callback_wrapper,
    callback);
  if (status)
    return status;
  status = CPXsetusercutcallbackfunc(p, impl::cpx::CBWrap::cut_callback_wrapper,
    callback);
  if (status)
    return status;
  status = CPXsetmipcallbackfunc(p, impl::cpx::CBWrap::lp_callback_wrapper, callback);
  if (status)
    return status;
  status = CPXsetlpcallbackfunc(p, impl::cpx::CBWrap::lp_callback_wrapper, callback);
  if (status)
    return status;
  status = CPXsetincumbentcallbackfunc(p, impl::cpx::CBWrap::incumbent_callback_wrapper, callback);
  if (status)
    return status;

  status = CPXsetheuristiccallbackfunc(p, impl::cpx::CBWrap::heuristiccallbackfunc_wrapper, callback);
  if (status)
    return status;

  return setMsgCallback(callback, p);
}

class MyCPLEXCallbackBridge : public CPLEXCallback {
  GenericCallback* cb_;
public:
  MyCPLEXCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* CPLEXModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyCPLEXCallbackBridge(callback);
}

int CPLEXModel::optimize() {
  CPXENVptr env = getCPXENV();
  int probtype = CPXgetprobtype(env, model_);
  int status = 0;
  switch (probtype)
  {
  case CPXPROB_LP:
    setParam(CPX_PARAM_LPMETHOD, CPX_ALG_AUTOMATIC);
    status = CPXlpopt(env, model_);
    break;
  case CPXPROB_MILP:
  case CPXPROB_FIXEDMILP:
  case CPXPROB_MIQP:
  case CPXPROB_FIXEDMIQP:
    status = CPXmipopt(env, model_);
    break;
  case CPXPROB_QP:
    status = CPXqpopt(env, model_);
    break;
  case CPXPROB_QCP:
  case CPXPROB_MIQCP:
    status = CPXhybbaropt(env, model_, CPX_ALG_NONE);
  }
  resetVarMapInternal();
  // This gets communicated to writeSol
  status_ = status;
  // Print error message in case of error
  AMPLSCPXERRORCHECK("CPX**opt");
  return status;
}

std::string CPLEXModel::error(int code) {
  char buffer[CPXMESSAGEBUFSIZE];
  CPXCCHARptr errstr;
  errstr = CPXgeterrorstring(this->getCPXENV(), code, buffer);

  if (errstr != NULL) {
    return std::string(buffer);
  }
  else {
    return "Error code not found.";
  }
}


std::vector<double>  CPLEXModel::getConstraintsValueImpl(int offset, int length) {
  std::vector<double> c(length);
  int status = CPXgetpi(getCPXENV(), getCPXLP(), c.data(), offset, offset + length);
  AMPLSCPXERRORCHECK("CPXgetx");
  return c;
}
std::vector<double> CPLEXModel::getVarsValueImpl(int offset, int length) {
  std::vector<double> c(length);
  int status = CPXgetx(getCPXENV(), getCPXLP(), c.data(), offset, offset + (length-1));
  AMPLSCPXERRORCHECK("CPXgetx");
  return c;
}

} // namespace