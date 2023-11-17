#include "gurobi_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr
namespace ampls
{
int impl::grb::callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata)
{
  GurobiCallback* cb = (GurobiCallback*)usrdata;
  cb->cbdata_ = cbdata;
  cb->where_ = where;

  cb->currentCapabilities_ = 0;
  if (where == GRB_CB_MIPNODE) 
    cb->currentCapabilities_ = ampls::CanDo::IMPORT_SOLUTION | CanDo::GET_LP_SOLUTION |
    ampls::CanDo::ADD_LAZY_CONSTRAINT | ampls::CanDo::ADD_USER_CUT;
  if (where == GRB_CB_MIPSOL)
    cb->currentCapabilities_ |= ampls::CanDo::ADD_LAZY_CONSTRAINT | 
    ampls::CanDo::IMPORT_SOLUTION | ampls::CanDo::GET_MIP_SOLUTION;
  if (where == GRB_CB_MIP)
    cb->currentCapabilities_ |= ampls::CanDo::IMPORT_SOLUTION;

  int res = cb->run();
  if (res == -1)
  {
    GRBterminate(model);
    return 0;
  }
  return res;
}

GurobiDrv::~GurobiDrv() {
}

GurobiModel GurobiDrv::loadModelImpl(char** args, const char** options) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::grb::AMPLSOpen_gurobi(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return GurobiModel(mp, args[1], options);
}

int GurobiModel::setCallbackDerived(impl::BaseCallback* callback) {
  return GRBsetcallbackfunc(GRBModel_, impl::grb::callback_wrapper, callback);
}

class MyGurobiCallbackBridge : public GurobiCallback {
  GenericCallback* cb_;
public:
  MyGurobiCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* GurobiModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyGurobiCallbackBridge(callback);
}

int GurobiModel::getIntAttr(const char* name) {
  int v;
  int r = GRBgetintattr(GRBModel_, name, &v);
  return v;
}
double GurobiModel::getDoubleAttr(const char* name) {
  double v;
  int r = GRBgetdblattr(GRBModel_, name, &v); 
  if (r != 0)
    return -1;
  return v;
}

int GurobiModel::getIntAttrArray(const char* name, int first, int length, int* arr) {
  return GRBgetintattrarray(GRBModel_, name, first, length, arr);
}
int GurobiModel::getDoubleAttrArray(const char* name, int first, int length, double* arr) {
  return GRBgetdblattrarray(GRBModel_, name, first, length, arr);
}

GurobiModel::~GurobiModel() {
  if (copied_)
    return;
  impl::grb::AMPLSClose_gurobi(solver_);
}
std::string GurobiModel::error(int code)
{
  return std::string(GRBgeterrormsg(getGRBenv()));
}


std::vector<double>  GurobiModel::getConstraintsValueImpl(int offset, int length) {
  std::vector<double> cons(length);
  int status = getDoubleAttrArray(GRB_DBL_ATTR_PI, offset, length, cons.data());
  AMPLSGRBERRORCHECK(status);
  return cons;
}
std::vector<double> GurobiModel::getVarsValueImpl(int offset, int length) {
  std::vector<double> vars(length);
  int status = getDoubleAttrArray(GRB_DBL_ATTR_X, offset, length, vars.data());
  AMPLSGRBERRORCHECK(status);
  return vars;
}


} // namespace