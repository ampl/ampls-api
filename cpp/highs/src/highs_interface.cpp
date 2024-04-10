#include "highs_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr


namespace ampls
{

  void  impl::highs::highs_callback_wrapper(const int where,
    const char* message, const HighsCallbackDataOut* dataout,
    HighsCallbackDataIn* datain, void* userdata)  {
    if (where == kHighsCallbackMipLogging)
      return;
    HighsCallback* cb = (HighsCallback*)userdata;
    cb->cbdata_ = dataout;
    cb->where_ = where;
    cb->msg_ = message;
    
    switch (where) {
    case kHighsCallbackMipSolution:
    case kHighsCallbackMipImprovingSolution:
      cb->currentCapabilities_ = CanDo::GET_MIP_SOLUTION | CanDo::ADD_LAZY_CONSTRAINT;
    default:
      cb->currentCapabilities_ = 0;
    }
    int res = cb->run();
    if (res != 0)
      datain->user_interrupt = 1;
  }


HighsDrv::~HighsDrv() {
}

HighsModel HighsDrv::loadModelImpl(char** args, const char** options) {
  return HighsModel(impl::highs::AMPLSOpen_highs(3, args), args[1], options);
}

int HighsModel::setCallbackDerived(impl::BaseCallback* callback) {
  int status = Highs_setCallback(HighsModel_, impl::highs::highs_callback_wrapper, callback);
  AMPLSHIGHSERRORCHECK(Highs_setCallback);
  Highs_startCallback(getHighsModel(), kHighsCallbackLogging);
  Highs_startCallback(getHighsModel(), kHighsCallbackSimplexInterrupt);
  Highs_startCallback(getHighsModel(), kHighsCallbackIpmInterrupt);
  Highs_startCallback(getHighsModel(), kHighsCallbackMipSolution);
  Highs_startCallback(getHighsModel(), kHighsCallbackMipImprovingSolution);
  Highs_startCallback(getHighsModel(), kHighsCallbackMipLogging);
  Highs_startCallback(getHighsModel(), kHighsCallbackMipInterrupt);
}

class MyHighsCallbackBridge : public HighsCallback {
  GenericCallback* cb_;
public:
  MyHighsCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* HighsModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyHighsCallbackBridge(callback);
}

int HighsModel::getIntAttr(const char* name) {
  int v;
  int r = Highs_getIntInfoValue(HighsModel_, name, &v);
  return v;
}
int64_t HighsModel::getInt64Attr(const char* name) {
  int64_t v;
  int r = Highs_getInt64InfoValue(HighsModel_, name, &v);
  return v;
}

double HighsModel::getDoubleAttr(const char* name) {
  double v;
  int r = Highs_getDoubleInfoValue(HighsModel_, name, &v);
  if (r != 0)
    return -1;
  return v;
}


HighsModel::~HighsModel() {
  if (copied_)
    return;
  impl::highs::AMPLSClose_highs(solver_);
}
std::string HighsModel::error(int code)
{
  return ""; // TODO
  
}

} // namespace