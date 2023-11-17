#include "cplex_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr

namespace ampls
{
 
int CPXPUBLIC impl::cpx::CBWrap::genericcallback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
    void* cbhandle) {
  CPLEXCallback* cb = static_cast<CPLEXCallback*>(cbhandle);
  CPXINT tid;

  if (cb->hasThreads_) 
    CPXcallbackgetinfoint(context, CPXCALLBACKINFO_THREADID, &tid);
  else
    tid = 0;

  // Assign thread local info
  cb->local_[tid].context_ = context;
  cb->local_[tid].where_ = (int)contextid;

  int res = cb->hasThreads_ ? cb->run(tid) : cb->run();
  if(res==-1)
    CPXcallbackabort(context);
  return 0;
}


bool impl::cpx::CBWrap::skipMsgCallback = false;
void CPXPUBLIC
impl::cpx::CBWrap::msg_callback_wrapper(void* handle, const char* msg)
{

  if (impl::cpx::CBWrap::skipMsgCallback)
    return;
  CPLEXCallback* cb = static_cast<CPLEXCallback*>(handle);
  
  cb->local_[0].where_ = CPX_ENUM_MSG_CALLBACK;
  cb->local_[0].context_ = nullptr;
  cb->msg_ = msg;
  if (cb->hasThreads_)
    cb->run(0);
  else
    cb->run();
}

CPLEXDrv::~CPLEXDrv() {
}

void CPLEXDrv::freeCPLEXEnv()
{
}

CPLEXModel CPLEXDrv::loadModelImpl(char** args, const char** options) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::cpx::AMPLSOpen_cplexmp(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return CPLEXModel(mp, args[1], options);
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

  int context = CPX_CALLBACKCONTEXT_BRANCHING | CPX_CALLBACKCONTEXT_CANDIDATE | CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS
    | CPX_CALLBACKCONTEXT_LOCAL_PROGRESS | CPX_CALLBACKCONTEXT_RELAXATION | CPX_CALLBACKCONTEXT_THREAD_UP | CPX_CALLBACKCONTEXT_THREAD_DOWN;

  int status = CPXcallbacksetfunc(getCPXENV(), getCPXLP(), context, impl::cpx::CBWrap::genericcallback, callback);

  if (status)
    return status;

  return setMsgCallback(callback, getCPXENV());
}

class MyCPLEXCallbackBridge : public CPLEXCallback {
  GenericCallback* cb_;
public:
  MyCPLEXCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return ((CPLEXCallback*)cb_)->run();
  }
  virtual int run(int i=0) {
    return ((CPLEXCallback*)cb_)->run(i);
  }
};

impl::BaseCallback* CPLEXModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyCPLEXCallbackBridge(callback);
}

void CPLEXModel::optimize() {
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
  AMPLSCPXERRORCHECK("CPX**opt");
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
  int status = CPXgetpi(getCPXENV(), getCPXLP(), c.data(), offset, offset + (length-1));
  AMPLSCPXERRORCHECK("CPXgetpi");
  return c;
}
std::vector<double> CPLEXModel::getVarsValueImpl(int offset, int length) {
  std::vector<double> c(length);
  int status = CPXgetx(getCPXENV(), getCPXLP(), c.data(), offset, offset + (length-1));
  AMPLSCPXERRORCHECK("CPXgetx");
  return c;
}

} // namespace