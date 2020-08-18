#ifndef CPLEX_INTERFACE_H_INCLUDE_
#define CPLEX_INTERFACE_H_INCLUDE_

#ifdef _WIN32
#define ENTRYPOINT __declspec(dllimport)
#else
#define ENTRYPOINT
#endif


#include <string>
#include <map>

#if CPX_APIMODEL == CPX_APIMODEL_LARGE
#include "ilcplex/cplexx.h"
#endif
#include "ilcplex/cplex.h"

#include "simpleapi/simpleApi.h"
#include "cplex_callback.h"

struct ASL;
struct CPLEXDriverState;


namespace cpx
{
  namespace impl
  {
    extern "C" {

      ENTRYPOINT CPLEXDriverState* AMPLCPLEXloadmodel(int argc, char** argv,
        CPXLPptr* modelPtr, ASL** aslPtr);

      ENTRYPOINT void AMPLCPLEXwritesol(CPLEXDriverState* state,
        CPXLPptr* modelPtr, int status);

      ENTRYPOINT CPXENVptr AMPLCPLEXgetInternalEnv();

      ENTRYPOINT void AMPLCPLEXfreeASL(ASL** aslPtr);
    }
  }
}

#ifdef _WIN32
#define CPXPUBLIC      __stdcall
#define CPXPUBVARARGS  __cdecl
#define CPXCDECL       __cdecl
#else
#define CPXPUBLIC
#define CPXPUBVARARGS
#define CPXCDECL
#endif


class CPLEXCallback;

int CPXPUBLIC lp_callback_wrapper(CPXCENVptr env, void* lp, int wf, void* cbh);
int CPXPUBLIC cut_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
  void* cbhandle, int* useraction_p);
void CPXPUBLIC msg_callback_wrapper(void* handle, const char* msg);
int CPXPUBLIC incumbent_callback_wrapper(CPXCENVptr env,
  void* cbdata, int wherefrom, void* cbhandle,
  double objval, double* x, int* isfeas_p,
  int* useraction_p);
CPLEXCallback* setDefaultCB(CPXCENVptr env, void* cbdata,
  int wherefrom, void* userhandle);
std::string getErrorMsg(CPXCENVptr env, int res);

class CPLEXModel;
class Callback;

/**
Encapsulates the main environment of the gurobi driver;
without modifications, a static CPLEXENV is created in the 
AMPL driver, and it would be fairly easy to lose track of it;
this way, it is deleted in the destructor.
*/
class CPLEXDrv {
  void freeCPLEXEnv();
  public:
    CPLEXModel loadModel(const char* modelName);
    CPXENVptr getEnv() {
      return cpx::impl::AMPLCPLEXgetInternalEnv();
    }
  ~CPLEXDrv();
};

/**
Encapsulates all the instance level information for a CPLEX model,
namely the CPLEX object, the relative ASL and all the locals of the 
driver up to the moment in which optimize would be called.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the relative structures.
*/
class CPLEXModel : public AMPLModel {
  friend CPLEXModel CPLEXDrv::loadModel(const char* modelName);

  mutable bool copied_;
  CPLEXDriverState* state_;
  int status_;
  CPXLPptr model_;
  ASL* asl_;
  int lastErrorCode_;
  CPLEXModel() : model_(NULL), asl_(NULL),
    lastErrorCode_(0), copied_(false), status_(0) {}

public:
  /*
  CPLEXModel(CPLEXModel&& other) noexcept :
    AMPLModel(other),
    state_(other.state_),
    asl_(other.asl_),
    model_(other.model_),
    lastErrorCode_(other.lastErrorCode_),
    copied_(false),
    status_(other.status_)
    {
      other.copied_ = true;
    }
  */
  CPLEXModel(const CPLEXModel& other) :
      AMPLModel(other)
    {
    state_ = other.state_;
    status_ = other.status_;
    copied_ = false;
    asl_ = other.asl_;
    model_ = other.model_;
    lastErrorCode_ = other.lastErrorCode_;
    fileName_ = other.fileName_;
    other.copied_ = true;
  }

  /* Write ampl solution file */
  void writeSol();

  int optimize();

  int getNumVars() {
    return CPXXgetnumcols(getCPLEXenv(), model_);
  }
  double getObj() {
    double obj;
    CPXgetobjval(getCPLEXenv(), model_, &obj);
    return obj;
  }

  int getSolution(int first, int length, double* sol) {
    return CPXgetx(getCPLEXenv(), model_, sol, first, length-1);
  }
  
  int setCallbackDerived(BaseCallback* callback);
  BaseCallback* createCallbackImplDerived(GenericCallback* callback);
  /*
  Get the map from variable name to index in the solver interface
  */
  std::map<std::string, int> getVarMap() {
    return getVarMapFiltered(NULL);
  }

  // Access to gurobi C structures
  CPXLPptr getCPXmodel() {
    return model_;
  }
  CPXENVptr getCPLEXenv() {
      return cpx::impl::AMPLCPLEXgetInternalEnv();
  }
  
  ~CPLEXModel() {
    if (copied_)
      return;
    if (model_)
      CPXfreeprob(getCPLEXenv(), &model_);
    if (asl_)
      cpx::impl::AMPLCPLEXfreeASL(&asl_);
  }
};
#endif // CPLEX_INTERFACE_H_INCLUDE_