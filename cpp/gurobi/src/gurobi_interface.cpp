#include "ampl_imports.h"

#include "gurobi_interface.h"
#include "gurobi_callback.h"


int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata)
{
  GRBCallback* cb = (GRBCallback*)usrdata;
  cb->cbdata_ = cbdata;
  cb->cbwhere_ = where;
  //int res = cb->run(cb->gurobiModel(), cbdata, where);
  int res = cb->run(where);
  return res;
}

GurobiDrv::~GurobiDrv() {
  freeGurobiEnv();
}

void GurobiDrv::freeGurobiEnv()
{
  grb::impl::freeEnvironmentPtr();
}

GurobiModel GurobiDrv::loadModel(const char* modelName) {
  grb::impl::initFunctions();
  char** args = generateArguments(modelName);
  GurobiModel m;
  try {
 
    m.GRBModel_ = grb::impl::AMPLloadmodelNoLicPtr(3, args, &m.asl_);
    m.lastErrorCode_ = -1;
    m.fileName_ = modelName;
  }
  catch (std::exception &e)
  {
    deleteParams(args);
    throw e;
  }
  deleteParams(args);
  return m;
}

void GurobiModel::writeSol() {
  grb::impl::AMPLwritesolPtr(GRBModel_, asl_, lastErrorCode_);
}
int GurobiModel::setCallbackDerived(BaseCallback* callback) {
  return GRBsetcallbackfunc(GRBModel_, callback_wrapper, callback);
}

class MyGurobiCallbackBridge : public GRBCallback {
  GenericCallback* cb_;
public:
  MyGurobiCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run(int where) {
    return cb_->run(where);
  }
};


BaseCallback* GurobiModel::createCallbackImplDerived(GenericCallback* callback) {
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
    grb::impl::freeASLPtr(&asl_);
}
