#include "cplex_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr

namespace ampls
{
CPLEXCallback* cpx::impl::CBWrap::setDefaultCB(CPXCENVptr env, void* cbdata,
  int wherefrom, void* userhandle)
{
  CPLEXCallback* cb = static_cast<CPLEXCallback*>(userhandle);
  cb->where_ = wherefrom;
  cb->env_ = env;
  cb->cbdata_ = cbdata;
  return cb;
}

int CPXPUBLIC cpx::impl::CBWrap::incumbent_callback_wrapper(CPXCENVptr env, void* cbdata,
  int wherefrom, void* userhandle,
  double objval, double* x, int* isfeas_p,
  int* useraction_p) {

  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle);

  *isfeas_p = 1; // use new solution by default
  cb->objval_ = objval;
  cb->x_ = x;
  if (cb->run())
    *useraction_p = CPX_CALLBACK_FAIL;
  else
    *useraction_p = CPX_CALLBACK_DEFAULT;
  return 0;
}


int CPXPUBLIC cpx::impl::CBWrap::lp_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
    void* userhandle)
{
  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle);
  return cb->run();
}

int CPXPUBLIC  cpx::impl::CBWrap::cut_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
  void* userhandle, int* useraction_p)
{
  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle);
  int res = cb->run();
  if (res)
    *useraction_p = CPX_CALLBACK_FAIL;
  else
    *useraction_p = CPX_CALLBACK_DEFAULT;
  return 0;
}


void CPXPUBLIC
cpx::impl::CBWrap::msg_callback_wrapper(void* handle, const char* msg)
{
  CPLEXCallback* cb = static_cast<CPLEXCallback*>(handle);
  cb->where_ = -1;
  cb->msg_ = msg;
  cb->run();
}

CPLEXDrv::~CPLEXDrv() {
}

void CPLEXDrv::freeCPLEXEnv()
{
  CPXENVptr env = getEnv();
  CPXcloseCPLEX(&env);
}


CPLEXModel* CPLEXDrv::loadModelImpl(char** args) {
  CPLEXModel* m = new CPLEXModel();
  CPXLPptr modelptr;
  ASL* aslptr;
  cpx::impl::CPLEXDriverState* state = cpx::impl::AMPLCPLEXloadmodel(3, args, &modelptr,
    &aslptr);
  if (state == NULL)
    throw AMPLSolverException::format("Trouble when loading model %s, most likely license-related.", args[1]);
  m->state_ = state;
  m->model_ = modelptr;
  m->asl_ = aslptr;
  m->lastErrorCode_ = -1;
  m->fileName_ = args[1];
  return m;
}
CPLEXModel CPLEXDrv::loadModel(const char* modelName) {
  std::unique_ptr<CPLEXModel> mod(loadModelGeneric(modelName));
  CPLEXModel c(*mod);
  return c;
}

void CPLEXModel::writeSolImpl(const char* solFileName) {
  cpx::impl::AMPLCPLEXwritesol(state_, model_, status_, solFileName);
}

int setMsgCallback(impl::BaseCallback* callback, CPXENVptr env) {
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

  status = CPXaddfuncdest(env, cpxerror, callback, cpx::impl::CBWrap::msg_callback_wrapper);
  if (status) {
    fprintf(stderr, "Could not set up error message handler.\n");
    CPXgeterrorstring(env, status, errmsg);
    fprintf(stderr, "%s\n", errmsg);
  }

  /* Now that we have the error message handler set up, all CPLEX
      generated errors will go through ourmsgfunc.  So we don't have
      to use CPXgeterrorstring to determine the text of the message. */

  status = CPXaddfuncdest(env, cpxwarning, callback, cpx::impl::CBWrap::msg_callback_wrapper);
  if (status) {
    cpx::impl::CBWrap::msg_callback_wrapper(callback, "Failed to set up handler for cpxwarning.\n");
    return 1;
  }

  status = CPXaddfuncdest(env, cpxresults, callback, cpx::impl::CBWrap::msg_callback_wrapper);
  if (status) {
    cpx::impl::CBWrap::msg_callback_wrapper(callback, "Failed to set up handler for cpxresults.\n");
    return 1;
  }
  return 0;
}

int CPLEXModel::setCallbackDerived(impl::BaseCallback* callback) {
  CPXENVptr p = getCPXENV();
  // Add the callback 
  int status = CPXsetlazyconstraintcallbackfunc(p, cpx::impl::CBWrap::cut_callback_wrapper,
    callback);
  if (status)
    return status;
  status = CPXsetusercutcallbackfunc(p, cpx::impl::CBWrap::cut_callback_wrapper,
    callback);
  if (status)
    return status;
  status = CPXsetmipcallbackfunc(p, cpx::impl::CBWrap::lp_callback_wrapper, callback);
  if (status)
    return status;
  status = CPXsetlpcallbackfunc(p, cpx::impl::CBWrap::lp_callback_wrapper, callback);
  if (status)
    return status;
  status = CPXsetincumbentcallbackfunc(p, cpx::impl::CBWrap::incumbent_callback_wrapper, callback);
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
} // namespace