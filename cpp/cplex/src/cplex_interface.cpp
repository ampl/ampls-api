#include "cplex_interface.h"
#include "simpleapi/simpleApi.h"
//#include <cstring>

namespace ampls
{
std::string getErrorMsg(CPXCENVptr env, int res) {
  char buffer[CPXMESSAGEBUFSIZE];
  CPXCCHARptr errstr = CPXgeterrorstring(env, res, buffer);
  if (errstr != NULL) {
    return buffer;
  }
  else {
    char CODE[60];
    sprintf(CODE, "CPLEX Error: %d. Unknown error code.", res);
    return CODE;
  }
}
CPLEXCallback* setDefaultCB(CPXCENVptr env, void* cbdata,
  int wherefrom, void* userhandle)
{
  CPLEXCallback* cb = static_cast<CPLEXCallback*>(userhandle);
  cb->where_ = wherefrom;
  cb->env_ = env;
  cb->cbdata_ = cbdata;
  return cb;
}

int CPXPUBLIC incumbent_callback_wrapper(CPXCENVptr env, void* cbdata,
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


int CPXPUBLIC
  lp_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
    void* userhandle)
{
  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle);
  return cb->run();
}

int CPXPUBLIC cut_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
  void* userhandle, int* useraction_p)
{
  CPLEXCallback* cb = setDefaultCB(env, cbdata, wherefrom, userhandle);
  if (cb->run())
    *useraction_p = CPX_CALLBACK_FAIL;
  else
    *useraction_p = CPX_CALLBACK_SET;
  return 0;
}


void CPXPUBLIC
  msg_callback_wrapper(void* handle, const char* msg)
{
  CPLEXCallback* cb = static_cast<CPLEXCallback*>(handle);
  cb->where_ = -1;
  cb->msg_ = msg;
  cb->run();
}


/* TODO: New-type callbacks don't work: they throw 1811 error when optimising,
even after disabling Dave's callbacks.
int CPXPUBLIC callback_wrapper(CPXCALLBACKCONTEXTptr context,
  CPXLONG contextid, void* userhandle)
{
  //Callback* cb = static_cast<Callback*>(userhandle);
  //userhandle->context_ = context;
  //userhandle->contextid_ = contextid;
  //return userhandle->run(context, contextid);
  return 0;
}
*/
CPLEXDrv::~CPLEXDrv() {
  freeCPLEXEnv();
}

void CPLEXDrv::freeCPLEXEnv()
{
  CPXENVptr env = getEnv();
  CPXcloseCPLEX(&env);
}

void disableCallbacksFromDave(CPXENVptr env) {
  CPXsetmipcallbackfunc(env, 0, 0);
  CPXsetlpcallbackfunc(env, 0, 0);
}
void CPLEXDrv::loadModelImpl(char** args, CPLEXModel* m) {
  
  try {
    CPXLPptr modelptr;
    ASL* aslptr;
    m->state_ = cpx::impl::AMPLCPLEXloadmodel(3, args, &modelptr,
      &aslptr);
    m->model_ = modelptr;
    disableCallbacksFromDave(cpx::impl::AMPLCPLEXgetInternalEnv());
    m->asl_ = aslptr;
    m->lastErrorCode_ = -1;
    m->fileName_ = args[1];
  }
  catch (...)
  {
  }
}
CPLEXModel CPLEXDrv::loadModel(const char* modelName) {
  
  char** args = generateArguments(modelName);
  CPLEXModel m;
  loadModelImpl(args, &m);
  return m;
  /*
  try {
    const std::lock_guard<std::mutex> lock(loadMutex);
    FILE* f = fopen(modelName, "rb");
    if (!f)
      throw ampls::AMPLSolverException("Could not find file: " + std::string(modelName));
    else
      fclose(f);
    CPXLPptr modelptr;
    ASL* aslptr;
    m.state_ = cpx::impl::AMPLCPLEXloadmodel(3, args, &modelptr,
      &aslptr);
    m.model_ = modelptr;
    disableCallbacksFromDave(cpx::impl::AMPLCPLEXgetInternalEnv());
    m.asl_ = aslptr;
    m.lastErrorCode_ = -1;
    m.fileName_ = modelName;
  }
  catch (ampls::AMPLSolverException& a)
  {
    deleteParams(args);
    throw a;
  }
  catch (std::exception& e)
  {
    deleteParams(args);
    throw e;
  }
  deleteParams(args);
  return m;*/
}

void CPLEXModel::writeSol() {
  cpx::impl::AMPLCPLEXwritesol(state_, model_, status_);
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

  status = CPXaddfuncdest(env, cpxerror, callback, msg_callback_wrapper);
  if (status) {
    fprintf(stderr, "Could not set up error message handler.\n");
    CPXgeterrorstring(env, status, errmsg);
    fprintf(stderr, "%s\n", errmsg);
  }

  /* Now that we have the error message handler set up, all CPLEX
      generated errors will go through ourmsgfunc.  So we don't have
      to use CPXgeterrorstring to determine the text of the message. */

  status = CPXaddfuncdest(env, cpxwarning, callback, msg_callback_wrapper);
  if (status) {
    msg_callback_wrapper(callback, "Failed to set up handler for cpxwarning.\n");
    return 1;
  }

  status = CPXaddfuncdest(env, cpxresults, callback, msg_callback_wrapper);
  if (status) {
    msg_callback_wrapper(callback, "Failed to set up handler for cpxresults.\n");
    return 1;
  }
  return 0;
}
int CPLEXModel::setCallbackDerived(impl::BaseCallback* callback) {
  CPXENVptr p = getCPLEXenv();

  // Add the callback 
  int status = CPXsetlazyconstraintcallbackfunc(p, cut_callback_wrapper,
    callback);
  if (status)
    return status;
  status = CPXsetusercutcallbackfunc(p, cut_callback_wrapper,
    callback);
  if (status)
    return status;
  status = CPXsetmipcallbackfunc(p, lp_callback_wrapper, callback);
  if (status)
    return status;

  status = CPXsetlpcallbackfunc(p, lp_callback_wrapper, callback);
  if (status)
    return status;
  status = CPXsetincumbentcallbackfunc(p, incumbent_callback_wrapper, callback);
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
  CPXENVptr env = getCPLEXenv();
  int probtype = CPXgetprobtype(env, model_);
  int res = 0;
  switch (probtype)
  {
  case CPXPROB_LP:
    printf("Calling lpopt\n");
    CPXsetintparam(env, CPX_PARAM_LPMETHOD, CPX_ALG_AUTOMATIC);
    res = CPXlpopt(env, model_);
    break;
  case CPXPROB_MILP:
  case CPXPROB_FIXEDMILP:
  case CPXPROB_MIQP:
  case CPXPROB_FIXEDMIQP:
    printf("Calling mipopt\n");
    res = CPXmipopt(env, model_);
    break;
  case CPXPROB_QP:
    printf("Calling qpopt\n");
    res = CPXqpopt(env, model_);
    break;
  case CPXPROB_QCP:
  case CPXPROB_MIQCP:
    printf("Calling hybbaropt\n");
    res = CPXhybbaropt(env, model_, CPX_ALG_NONE);
  }
  resetVarMapInternal();
  // This gets communicated to writeSol
  status_ = res;
  // Print error message in case of error
  if (res)
  {
    char buffer[CPXMESSAGEBUFSIZE];
    CPXCCHARptr errstr = CPXgeterrorstring(env, res, buffer);
    if (errstr != NULL) {
      printf("%s \n", buffer);
    }
    else {
      printf("CPLEX Error %5d: Unknown error code. \n",
        res);
    }
    return res;
  }
  return res;
}

std::string CPLEXModel::error(int code) {
  char buffer[CPXMESSAGEBUFSIZE];
  CPXCCHARptr errstr;
  errstr = CPXgeterrorstring(this->getCPLEXenv(), code, buffer);

  if (errstr != NULL) {
    return std::string(buffer);
  }
  else {
    return "Error code not found.";
  }
}
} // namespace