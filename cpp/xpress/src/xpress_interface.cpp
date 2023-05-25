#include "xpress_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr

namespace ampls
{

namespace impl {
  namespace xpress {
XPRESSCallback* XPRSCBWrap::setDefaultCB(XPRSprob prob, void* data,
  XPRESSWhere wherefrom, int capabilities)
{
  XPRESSCallback* cb = static_cast<XPRESSCallback*>(data);
  cb->where_ = (int)wherefrom;
  cb->prob_ = prob;
  cb->currentCapabilities_ = capabilities;
  return cb;
}

void  XPRSCBWrap::message_callback_wrapper(XPRSprob prob, void* object, const char* msg, int len, int msgtype)
{
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::message);
  cb->msg_ = msg;
  if(msg != NULL)
    cb->run();
}
void XPRS_CC XPRSCBWrap::intsol_callback_wrapper(XPRSprob prob, void* object)
{
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::intsol, CanDo::IMPORT_SOLUTION | CanDo::GET_LP_SOLUTION);
  cb->run();
}

void XPRS_CC XPRSCBWrap::optnode_callback_wrapper(XPRSprob prob, void* object, int* feas)
{
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::optnode);
  cb->run();
}

} // impl
} // xpress


XPRESSDrv::~XPRESSDrv() {
}


XPRESSModel XPRESSDrv::loadModelImpl(char** args) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::xpress::AMPLSOpen_xpress(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return XPRESSModel(mp, args[1]);
}
XPRESSModel XPRESSDrv::loadModel(const char* modelName) {
  return loadModelGeneric(modelName);
}

int XPRESSModel::setCallbackDerived(impl::BaseCallback* callback) {
   
  // Add the callbacks
  int status = XPRSsetcbintsol(prob_, impl::xpress::XPRSCBWrap::intsol_callback_wrapper,
    callback);
  AMPLSXPRSERRORCHECK("XPRSsetcbintsol")
  status = XPRSsetcbmessage(prob_, impl::xpress::XPRSCBWrap::message_callback_wrapper,
    callback);
  AMPLSXPRSERRORCHECK("XPRSsetcbmessage")
  status = XPRSsetcboptnode(prob_, impl::xpress::XPRSCBWrap::optnode_callback_wrapper,
    callback);
   AMPLSXPRSERRORCHECK("XPRSsetcboptnode")
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
  tStart_ = clock();
  if (getIntAttr(XPRS_ORIGINALMIPENTS) > 0)
    return XPRSmipoptimize(prob_, NULL);
  else
    return XPRSlpoptimize(prob_, NULL);
}

std::vector<double> XPRESSModel::getConstraintsValueImpl(int offset, int length) {
  std::vector<double> cons(getNumCons());
  int status = XPRSgetsol(prob_, NULL, NULL, cons.data(), NULL);
  AMPLSXPRSERRORCHECK("XPRSgetsol")
  std::vector<double> toRet(cons.begin() + offset, cons.begin() + offset + length);
  return toRet;
}
std::vector<double> XPRESSModel::getVarsValueImpl(int offset, int length) {
  
  auto vars = getSolutionVector();
  std::vector<double> toRet(vars.begin() + offset, vars.begin() + offset + length);
  return toRet;
}
} // namespace