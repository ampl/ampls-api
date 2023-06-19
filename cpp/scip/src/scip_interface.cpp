#include <functional>
#include <memory> // for unique_ptr

#include "scip_interface.h"
#include "ampls/ampls.h"

namespace ampls
{
 


void impl::scip::cut_callback_wrapper(void* model, void* osicuts, void* appdata, int level, int pass)
{
  SCIPCallback* cb = (SCIPCallback*)appdata;
  cb->where_ = ampls::Where::MIPSOL;
  cb->model_ = model;
  cb->osicuts_ = osicuts;
  cb->run();
}

void impl::scip::callback_wrapper(SCIP* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
  int nchar, char** cvec) {
  //cb->where_ = ampls::Where::MSG;
}


SCIPDrv::~SCIPDrv() {
}

SCIPModel SCIPDrv::loadModelImpl(char** args, const char** options) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::scip::AMPLSOpen_scip(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return SCIPModel(mp, args[1], options);
}

void SCIPModel::writeSolImpl(const char* solFileName) {
  impl::mp::AMPLSReportResults(solver_, solFileName);
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


class MySCIPCallbackBridge : public SCIPCallback {
  GenericCallback* cb_;
public:
  MySCIPCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* SCIPModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MySCIPCallbackBridge(callback);
}

void SCIPModel::optimize() {
  SCIPsolve(model_);
}


SCIPModel::~SCIPModel() {
  if (copied_)
    return;
  impl::scip::AMPLSClose_scip(model_);
}

std::string SCIPModel::error(int code)
{ //TODO
  return "";
  //return std::string(GRBgeterrormsg(getGRBenv()));
}


std::vector<double> SCIPModel::getConstraintsValueImpl(int offset, int length) {
  // TODO
  std::vector<double> cons(length);
  return cons;
}
std::vector<double> SCIPModel::getVarsValueImpl(int offset, int length) {
  
  //auto variables = Cbc_getColSolution(model_);
  //std::vector<double> vars(variables, variables + getNumVars());
  //return vars;
}


} // namespace