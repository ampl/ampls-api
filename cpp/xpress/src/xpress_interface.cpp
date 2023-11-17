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
  cb->preintsol_ = 0;
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
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::intsol, 
    CanDo::IMPORT_SOLUTION | CanDo::GET_LP_SOLUTION);
  int ret = cb->run();
  if (ret == -1) XPRSinterrupt(prob, XPRS_STOP_USER);
}

void XPRS_CC XPRSCBWrap::newnode_callback_wrapper(XPRSprob prob, void* object, int parentnode, int node, int branch)
{
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::newnode);
  int ret = cb->run();
  if (ret == -1) XPRSinterrupt(prob, XPRS_STOP_USER);
}


void XPRS_CC XPRSCBWrap::optnode_callback_wrapper(XPRSprob prob, void* object, int* feas)
{
  int v;
  XPRSgetintattrib(prob, XPRS_MIPINFEAS, &v);
  auto where = v > 0 ? XPRESSWhere::optnode : XPRESSWhere::intsol;
  XPRESSCallback* cb = setDefaultCB(prob, object, where,
    CanDo::IMPORT_SOLUTION | CanDo::GET_LP_SOLUTION | CanDo::ADD_LAZY_CONSTRAINT | CanDo::ADD_USER_CUT);
  cb->feas_ = *feas;
  int ret = cb->run();
  *feas = 0;
  if (ret == -1) XPRSinterrupt(prob, XPRS_STOP_USER);
}

void XPRS_CC XPRSCBWrap::preintsol_callback_wrapper(XPRSprob prob, void* object, int soltype, int* p_reject, double* p_cutoff)
{
  XPRESSCallback* cb = setDefaultCB(prob, object, XPRESSWhere::intsol,
    CanDo::IMPORT_SOLUTION | CanDo::GET_LP_SOLUTION | CanDo::ADD_LAZY_CONSTRAINT | CanDo::ADD_USER_CUT);
  cb->preintsol_ = 1;
  cb->feas_ = *p_reject;
  int ret=cb->run();
  *p_reject = cb->feas_;
  if (ret == -1) XPRSinterrupt(prob, XPRS_STOP_USER);

}
} // impl
} // xpress


XPRESSDrv::~XPRESSDrv() { }

XPRESSModel XPRESSDrv::loadModelImpl(char** args, const char** options) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::xpress::AMPLSOpen_xpress(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return XPRESSModel(mp, args[1], options);
}
int XPRESSModel::setCallbackDerived(impl::BaseCallback* callback) {
  int status = XPRSsetcbintsol(prob_, impl::xpress::XPRSCBWrap::intsol_callback_wrapper,
    callback);
  AMPLSXPRSERRORCHECK("XPRSsetcbintsol")
  status = XPRSaddcbpreintsol(prob_, impl::xpress::XPRSCBWrap::preintsol_callback_wrapper,
    callback, 0);
  AMPLSXPRSERRORCHECK("XPRSsetcbpreintsol")
  status = XPRSsetcbmessage(prob_, impl::xpress::XPRSCBWrap::message_callback_wrapper,
    callback);
  AMPLSXPRSERRORCHECK("XPRSsetcbmessage")
  status = XPRSsetcboptnode(prob_, impl::xpress::XPRSCBWrap::optnode_callback_wrapper,
    callback);
  AMPLSXPRSERRORCHECK("XPRSsetcboptnode")
    status = XPRSsetcbnewnode(prob_, impl::xpress::XPRSCBWrap::newnode_callback_wrapper,
      callback);
  AMPLSXPRSERRORCHECK("XPRSsetcbnewnode")
  return status;
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