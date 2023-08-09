#include "copt_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr
namespace ampls
{

  int COPT_CALL impl::copt::copt_callback_wrapper(copt_prob* prob, void* cbdata, int cbctx, void* userdata)
{
  CoptCallback* cb = (CoptCallback*)userdata;
  cb->cbdata_ = cbdata;
  cb->where_ = cbctx;
  int common = CanDo::IMPORT_SOLUTION | CanDo::ADD_LAZY_CONSTRAINT | CanDo::ADD_USER_CUT | CanDo::IMPORT_SOLUTION;
  if (cb->getAMPLWhere() == ampls::Where::MIPNODE)
    cb->currentCapabilities_ = CanDo::GET_LP_SOLUTION | common;
  else
    cb->currentCapabilities_ = common;

  int res = cb->run();
  if (res!=0)
  {
    COPT_Interrupt(prob);
    return res;
  }
  return res;
}
 void COPT_CALL impl::copt::copt_log_callback_wrapper(char* msg, void* userdata) {
  CoptCallback* cb = (CoptCallback*)userdata;
  cb->where_ = ampls::Where::MSG; 
  cb->cbdata_ = (void*)msg;
  cb->currentCapabilities_ = 0;
  int res = cb->run();
}

CoptDrv::~CoptDrv() {
}

CoptModel CoptDrv::loadModelImpl(char** args, const char** options) {
  return CoptModel(impl::copt::AMPLSOpen_copt(3, args), args[1], options);
}

int CoptModel::setCallbackDerived(impl::BaseCallback* callback) {
  COPT_SetCallback(COPTModel_, impl::copt::copt_callback_wrapper, COPT_CBCONTEXT_MIPRELAX, callback);
  COPT_SetCallback(COPTModel_, impl::copt::copt_callback_wrapper, COPT_CBCONTEXT_MIPSOL, callback);
  COPT_SetLogCallback(COPTModel_, impl::copt::copt_log_callback_wrapper, callback);
  return 0;
}

class MyCoptCallbackBridge : public CoptCallback {
  GenericCallback* cb_;
public:
  MyCoptCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* CoptModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyCoptCallbackBridge(callback);
}

int CoptModel::getIntAttr(const char* name) {
  int v;
  int r = COPT_GetIntAttr(COPTModel_, name, &v);
  return v;
}
double CoptModel::getDoubleAttr(const char* name) {
  double v;
  int r = COPT_GetDblAttr(COPTModel_, name, &v);
  if (r != 0)
    return -1;
  return v;
}


CoptModel::~CoptModel() {
  if (copied_)
    return;
  impl::copt::AMPLSClose_copt(solver_);
}
std::string CoptModel::error(int code)
{
  return ""; // TODO
  
}

} // namespace