#include "cbcmp_interface.h"

#include "ampls/ampls.h"

#include <memory> // for unique_ptr
namespace ampls
{


void impl::cbcmp::callback_wrapper(void* osisolver, void* osicuts, void* appdata)
{
  printf("I am here");
  
  //CbcCallback* cb = (CbcCallback*)usrdata;
  /*
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
  return res;*/
}

CbcDrv::~CbcDrv() {
}

CbcModel CbcDrv::loadModelImpl(char** args) {
  return CbcModel(impl::cbcmp::AMPLSOpen_cbcmp(3, args), args[1]);
}
CbcModel CbcDrv::loadModel(const char* modelName) {
  return loadModelGeneric(modelName);
}

void CbcModel::writeSolImpl(const char* solFileName) {
  impl::mp::AMPLSReportResults(solver_, solFileName);
}
int CbcModel::setCallbackDerived(impl::BaseCallback* callback) {
  Cbc_addCutCallback(model_, (cbc_cut_callback)impl::cbcmp::callback_wrapper, "amplscallback", callback);
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

int CbcModel::optimize() {
  int s = Cbc_solve(model_);
  resetVarMapInternal();
  return s;
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