#include "x-gurobi_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr
namespace ampls
{
int xgrb::impl::callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata)
{
  XGurobiCallback* cb = (XGurobiCallback*)usrdata;
  cb->cbdata_ = cbdata;
  cb->where_ = where;
  if (cb->getAMPLWhere() == ampls::Where::MIPNODE)
    cb->currentCapabilities_ = ampls::CanDo::IMPORT_SOLUTION | CanDo::GET_LP_SOLUTION;
  else
    cb->currentCapabilities_ = 0;

  int res = cb->run();
  if (res == -1)
  {
    GRBterminate(model);
    return 0;
  }
  return res;
}

XGurobiDrv::~XGurobiDrv() {
}

XGurobiModel XGurobiDrv::loadModelImpl(char** args) {
  XGurobiModel m;
  void *s = xgrb::impl::AMPLloadmodel(3, args);
  if (s == NULL)
    throw AMPLSolverException::format("Trouble when loading model %s.", args[1]);
  GRBmodel* inner = xgrb::impl::AMPLgetGRBModel(s);
  m.solver_ = s;
  m.GRBModel_ = inner;
  m.lastErrorCode_ = -1;
  m.fileName_ = args[1];
  return m;
}
XGurobiModel XGurobiDrv::loadModel(const char* modelName) {
  return loadModelGeneric(modelName);
}

void XGurobiModel::writeSolImpl(const char* solFileName) {
  xgrb::impl::AMPLSReportResults(solver_, solFileName);
}
int XGurobiModel::setCallbackDerived(impl::BaseCallback* callback) {
  return GRBsetcallbackfunc(GRBModel_, xgrb::impl::callback_wrapper, callback);
}

class MyXGurobiCallbackBridge : public XGurobiCallback {
  GenericCallback* cb_;
public:
  MyXGurobiCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* XGurobiModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyXGurobiCallbackBridge(callback);
}

int XGurobiModel::optimize() {
  lastErrorCode_ = GRBoptimize(GRBModel_);
  resetVarMapInternal();
  return lastErrorCode_;
}

int XGurobiModel::getIntAttr(const char* name) {
  int v;
  int r = GRBgetintattr(GRBModel_, name, &v);
  return v;
}
double XGurobiModel::getDoubleAttr(const char* name) {
  double v;
  int r = GRBgetdblattr(GRBModel_, name, &v); 
  if (r != 0)
    return -1;
  return v;
}

int XGurobiModel::getIntAttrArray(const char* name, int first, int length, int* arr) {
  return GRBgetintattrarray(GRBModel_, name, first, length, arr);
}
int XGurobiModel::getDoubleAttrArray(const char* name, int first, int length, double* arr) {
  return GRBgetdblattrarray(GRBModel_, name, first, length, arr);
}

XGurobiModel::~XGurobiModel() {
  if (copied_)
    return;
  xgrb::impl::AMPLclosesolver(solver_);
}
std::string XGurobiModel::error(int code)
{
  return std::string(GRBgeterrormsg(getGRBenv()));
}


std::vector<double>  XGurobiModel::getConstraintsValueImpl(int offset, int length) {
  std::vector<double> cons(length);
  int status = getDoubleAttrArray(GRB_DBL_ATTR_PI, offset, length, cons.data());
  AMPLSGRBERRORCHECK(status);
  return cons;
}
std::vector<double> XGurobiModel::getVarsValueImpl(int offset, int length) {
  std::vector<double> vars(length);
  int status = getDoubleAttrArray(GRB_DBL_ATTR_X, offset, length, vars.data());
  AMPLSGRBERRORCHECK(status);
  return vars;
}


} // namespace