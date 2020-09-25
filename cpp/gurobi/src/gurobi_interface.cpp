#include "gurobi_interface.h"
#include "gurobi_callback.h"

namespace ampls
{
int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata)
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

GurobiModel GurobiDrv::loadModel(const char* modelName) {
  char** args = generateArguments(modelName);
  GurobiModel m;
  try {
  FILE* f = fopen(modelName, "rb");
  if (!f)
    throw ampls::AMPLSolverException("Could not find file: " + std::string(modelName));
  else
    fclose(f);
  const std::lock_guard<std::mutex> lock(loadMutex);
    m.GRBModel_ = grb::impl::AMPLloadmodelNoLic(3, args, &m.asl_);
    m.lastErrorCode_ = -1;
    m.fileName_ = modelName;
  }
  catch (std::exception& e)
  {
    deleteParams(args);
    throw e;
  }
  deleteParams(args);
  return m;
}

void GurobiModel::writeSol() {
  grb::impl::AMPLwritesol(GRBModel_, asl_, lastErrorCode_);
}
int GurobiModel::setCallbackDerived(impl::BaseCallback* callback) {
  return GRBsetcallbackfunc(GRBModel_, callback_wrapper, callback);
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