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
  int res = cb->run();
  return res;
}

GurobiDrv::~GurobiDrv() {
  freeGurobiEnv();
}

void GurobiDrv::freeGurobiEnv()
{
  grb::impl::freeEnvironment();
}
GurobiModel* GurobiDrv::loadModelImpl(char** args) {
  GurobiModel* m = new GurobiModel();
  GRBmodel* inner= grb::impl::AMPLloadmodel(3, args, &m->asl_);
  if (inner == NULL)
  {
    delete m;
    throw AMPLSolverException::format("Trouble when loading model %s, most likely license-related.", args[1]);
  }
  m->GRBModel_ = inner;
  m->lastErrorCode_ = -1;
  m->fileName_ = args[1];
  return m;
}
GurobiModel GurobiDrv::loadModel(const char* modelName) {
  std::unique_ptr<GurobiModel> mod(loadModelGeneric(modelName));
  GurobiModel c(*mod);
  return c;
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

} // namespace