#include "xpress_interface.h"
#include "simpleapi/simpleApi.h"

namespace ampls
{
namespace xpress {
namespace impl {
XPRESSCallback* CBWrap::setDefaultCB(XPRSprob prob, void* data,
  XPRESSWhere wherefrom)
{
  XPRESSCallback* cb = static_cast<XPRESSCallback*>(data);
  cb->where_ = (int)wherefrom;
  cb->prob_ = prob;
  return cb;
}

void  CBWrap::message_callback_wrapper(XPRSprob prob, void* object, const char* msg, int len, int msgtype)
{
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::message);
  cb->msg_ = msg;
  cb->run();
}
void XPRS_CC CBWrap::intsol_callback_wrapper(XPRSprob prob, void* object)
{
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::intsol);
  cb->run();
}



} // impl
} // xpress


XPRESSDrv::~XPRESSDrv() {
  xpress::impl::AMPLXPRESSfreeEnv();
}


XPRESSModel* XPRESSDrv::loadModelImpl(char** args) {
  XPRESSModel* m = new XPRESSModel();
  XPRSprob prob;
  xpress::impl::XPressDriverState *s = 
    xpress::impl::AMPLXPRESSloadModel(3, args, &m->prob_);
  m->fileName_ = args[1];
  return m;
}
XPRESSModel XPRESSDrv::loadModel(const char* modelName) {
  std::auto_ptr<XPRESSModel> mod = loadModelGeneric(modelName);
  XPRESSModel c(*mod);
  return c;
}

void XPRESSModel::writeSol() {
  xpress::impl::AMPLXPRESSwriteSolution(state_, prob_);
}

int XPRESSModel::setCallbackDerived(impl::BaseCallback* callback) {
   
  // Add the callbacks
  int status = XPRSsetcbintsol(prob_, xpress::impl::CBWrap::intsol_callback_wrapper, 
    callback);
  if (status)
    return status;


  status = XPRSsetcbmessage(prob_, xpress::impl::CBWrap::message_callback_wrapper,
    callback);
  if (status)
    return status;

  // TODO Finish!
}

class MyXPRESSCallbackBridge : public XPRESSCallback {
  GenericCallback* cb_;
public:
  MyXPRESSCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* XPRESSModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyXPRESSCallbackBridge(callback);
}

int XPRESSModel::optimize() {
  if (getInt(XPRS_ORIGINALMIPENTS) > 0)
    return XPRSmipoptimize(prob_, NULL);
  else
    return XPRSlpoptimize(prob_, NULL);

}
} // namespace