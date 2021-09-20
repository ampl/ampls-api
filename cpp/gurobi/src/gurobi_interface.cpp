#include "gurobi_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr
namespace ampls
{
int grb::impl::callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata)
{
  GurobiCallback* cb = (GurobiCallback*)usrdata;
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

GurobiDrv::~GurobiDrv() {
  freeGurobiEnv();
}

void GurobiDrv::freeGurobiEnv()
{
  grb::impl::freeEnvironment();
}
GurobiModel GurobiDrv::loadModelImpl(char** args) {
  GurobiModel m;
  GRBmodel* inner= grb::impl::AMPLloadmodel(3, args, &m.asl_);
  if (inner == NULL)
  {
    const char* error = grb::impl::getUinfo(m.asl_);
    if (error)
      throw AMPLSolverException::format("Trouble when loading model %s:\n%s", args[1], error);
    else
      throw AMPLSolverException::format("Trouble when loading model %s.", args[1]);
  }
  m.GRBModel_ = inner;
  m.lastErrorCode_ = -1;
  m.fileName_ = args[1];
  return m;
}
GurobiModel GurobiDrv::loadModel(const char* modelName) {
  return loadModelGeneric(modelName);
}

void GurobiModel::writeSolImpl(const char* solFileName) {
  grb::impl::AMPLwritesol(GRBModel_, asl_, lastErrorCode_, solFileName);
}
int GurobiModel::setCallbackDerived(impl::BaseCallback* callback) {
  return GRBsetcallbackfunc(GRBModel_, grb::impl::callback_wrapper, callback);
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

int GurobiModel::optimize() {
  lastErrorCode_ = GRBoptimize(GRBModel_);
  resetVarMapInternal();
  return lastErrorCode_;
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
  if (GRBModel_)
    GRBfreemodel(GRBModel_);
  if (asl_)
    grb::impl::freeASL(&asl_);
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