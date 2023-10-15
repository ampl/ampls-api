#include <functional>
#include <memory> // for unique_ptr

#include "cbcmp_interface.h"
#include "ampls/ampls.h"

namespace ampls
{
 


void impl::cbcmp::cut_callback_wrapper(void* osisolver, void* osicuts, void* appdata, int level, int pass)
{
  CbcCallback* cb = (CbcCallback*)appdata;
  cb->where_ = ampls::Where::MIPSOL;
  cb->osisolver_ = osisolver;
  cb->osicuts_ = osicuts;
  cb->run();
}

void impl::cbcmp::callback_wrapper(Cbc_Model* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
  int nchar, char** cvec) {
  //cb->where_ = ampls::Where::MSG;

}


CbcDrv::~CbcDrv() {
}

CbcModel CbcDrv::loadModelImpl(char** args, const char** options) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::cbcmp::AMPLSOpen_cbcmp(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return CbcModel(mp, args[1], options);
}




template <typename T>
struct MsgCallback;

template <typename Ret, typename... Params>
struct MsgCallback<Ret(Params...)> {
  template <typename... Args>
  static Ret callback(Args... args) {
    return func(args...);
  }
  static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> MsgCallback<Ret(Params...)>::func;

int CbcModel::setCallbackDerived(impl::BaseCallback* callback) {
  Cbc_addCutCallback(model_, (cbc_cut_callback)impl::cbcmp::cut_callback_wrapper, "amplscallback", callback);
  CbcCallback* cbcc = dynamic_cast<CbcCallback*>(callback);
  MsgCallback<void(Cbc_Model*,int,int,const double*,int,const int*,int,char**)>::func = 
    std::bind(&CbcCallback::call_msg_callback, cbcc, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7,
      std::placeholders::_8);
  cbc_callback func = static_cast<cbc_callback>(MsgCallback<void(Cbc_Model*, int, int, const double*, int, const int*, int, char**)>::callback);
  Cbc_registerCallBack(model_, func);
  return 0;
}

class MyCbcCallbackBridge : public CbcCallback {
  GenericCallback* cb_;
public:
  MyCbcCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* CbcModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyCbcCallbackBridge(callback);
}

void CbcModel::optimize() {
  Cbc_solve(model_);
  resetVarMapInternal();
}

int CbcModel::getIntAttr(const char* name) {
  int v = 0;
  //TODO
  //int r = GRBgetintattr(GRBModel_, name, &v);
  return v;
}
double CbcModel::getDoubleAttr(const char* name) {
  double v = 0;
  //TODO
  //int r = GRBgetdblattr(GRBModel_, name, &v); 
  //if (r != 0)
  //  return -1;
  return v;
}

int CbcModel::getIntAttrArray(const char* name, int first, int length, int* arr) {
  //TODO
  //return GRBgetintattrarray(GRBModel_, name, first, length, arr);
  return 0;
}
int CbcModel::getDoubleAttrArray(const char* name, int first, int length, double* arr) {
  //TODO
  //return GRBgetdblattrarray(GRBModel_, name, first, length, arr);
  return 0;
}

CbcModel::~CbcModel() {
  if (copied_)
    return;
  // TODO: this crashes!
  //cbcmp::impl::AMPLclosesolver(solver_);
}
std::string CbcModel::error(int code)
{ //TODO
  return "";
  //return std::string(GRBgeterrormsg(getGRBenv()));
}


std::vector<double>  CbcModel::getConstraintsValueImpl(int offset, int length) {
  // TODO
  std::vector<double> cons(length);
  return cons;
}
std::vector<double> CbcModel::getVarsValueImpl(int offset, int length) {
  
  auto variables = Cbc_getColSolution(model_);
  std::vector<double> vars(variables, variables + getNumVars());
  return vars;
}


} // namespace