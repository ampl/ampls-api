#include "gurobidirect_interface.h"
#include "ampls/ampls.h"

#include <memory> // for unique_ptr
namespace ampls
{
int grbdirect::impl::callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata)
{
  GurobiDirectCallback* cb = (GurobiDirectCallback*)usrdata;
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

GurobiDirectDrv::~GurobiDirectDrv() {
  freeGurobiEnv();
}

void GurobiDirectDrv::freeGurobiEnv()
{
 
}
GurobiDirectModel GurobiDirectDrv::loadModelImpl(char** args) {
  GurobiDirectModel m;
  void *s;
  GRBmodel* inner= grbdirect::impl::AMPLdirectloadmodel(3, args, &s);
  if (inner == NULL)
  {
   // const char* error = grb::impl::AMPLGRBgetUinfo(a);
    //if (error)
   //   throw AMPLSolverException::format("Trouble when loading model %s:\n%s", args[1], error);
   // else
   //   throw AMPLSolverException::format("Trouble when loading model %s.", args[1]);
  }
  m.solver_ = s;
  m.GRBModel_ = inner;
  m.lastErrorCode_ = -1;
  m.fileName_ = args[1];
  return m;
}
GurobiDirectModel GurobiDirectDrv::loadModel(const char* modelName) {
  return loadModelGeneric(modelName);
}

void GurobiDirectModel::writeSolImpl(const char* solFileName) {
  grbdirect::impl::AMPLdirectwritesolution(solver_);
}
int GurobiDirectModel::setCallbackDerived(impl::BaseCallback* callback) {
  return GRBsetcallbackfunc(GRBModel_, grbdirect::impl::callback_wrapper, callback);
}

class MyGurobiDirectCallbackBridge : public GurobiDirectCallback {
  GenericCallback* cb_;
public:
  MyGurobiDirectCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* GurobiDirectModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MyGurobiDirectCallbackBridge(callback);
}

int GurobiDirectModel::optimize() {
  lastErrorCode_ = GRBoptimize(GRBModel_);
  resetVarMapInternal();
  return lastErrorCode_;
}

int GurobiDirectModel::getIntAttr(const char* name) {
  int v;
  int r = GRBgetintattr(GRBModel_, name, &v);
  return v;
}
double GurobiDirectModel::getDoubleAttr(const char* name) {
  double v;
  int r = GRBgetdblattr(GRBModel_, name, &v); 
  if (r != 0)
    return -1;
  return v;
}

int GurobiDirectModel::getIntAttrArray(const char* name, int first, int length, int* arr) {
  return GRBgetintattrarray(GRBModel_, name, first, length, arr);
}
int GurobiDirectModel::getDoubleAttrArray(const char* name, int first, int length, double* arr) {
  return GRBgetdblattrarray(GRBModel_, name, first, length, arr);
}

GurobiDirectModel::~GurobiDirectModel() {
  if (copied_)
    return;
  grbdirect::impl::AMPLdirectclosesolver(solver_);
//  if (GRBModel_)
 //   GRBfreemodel(GRBModel_);
}
std::string GurobiDirectModel::error(int code)
{
  return std::string(GRBgeterrormsg(getGRBenv()));
}


std::vector<double>  GurobiDirectModel::getConstraintsValueImpl(int offset, int length) {
  std::vector<double> cons(length);
  int status = getDoubleAttrArray(GRB_DBL_ATTR_PI, offset, length, cons.data());
  AMPLSGRBERRORCHECK(status);
  return cons;
}
std::vector<double> GurobiDirectModel::getVarsValueImpl(int offset, int length) {
  std::vector<double> vars(length);
  int status = getDoubleAttrArray(GRB_DBL_ATTR_X, offset, length, vars.data());
  AMPLSGRBERRORCHECK(status);
  return vars;
}


} // namespace