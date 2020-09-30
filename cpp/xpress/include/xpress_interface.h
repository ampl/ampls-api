#ifndef CPLEX_INTERFACE_H_INCLUDE_
#define CPLEX_INTERFACE_H_INCLUDE_

#ifdef _WIN32
#define ENTRYPOINT __declspec(dllimport)
#else
#define ENTRYPOINT
#endif


#include <string>
#include <map>
#include <mutex>

#include "xprs.h"

#include "simpleapi/simpleApi.h"
#include "xpress_callback.h"


struct ASL;


namespace ampls
{

namespace xpress
{
  namespace impl
  {
    struct XPressDriverState;
    extern "C" {
      ENTRYPOINT void AMPLXPRESSfreeEnv();

      ENTRYPOINT XPressDriverState* AMPLXPRESSloadModel(int argc, char** argv,
        XPRSprob* modelPtr);

      ENTRYPOINT void AMPLXPRESSwriteSolution(XPressDriverState* state,
        XPRSprob modelPtr);
    }
  }
}

class XPRESSCallback;
class XPRESSModel;

/**
Encapsulates the main environment of the gurobi driver;
without modifications, a static CPLEXENV is created in the
AMPL driver, and it would be fairly easy to lose track of it;
this way, it is deleted in the destructor.
*/
class XPRESSDrv : public impl::SolverDriver<XPRESSModel> {
  void freeXPRESSEnv();
  XPRESSModel* loadModelImpl(char** args);
public:
  XPRESSModel loadModel(const char* modelName);
  ~XPRESSDrv();
};

/**
Encapsulates all the instance level information for a CPLEX model,
namely the CPLEX object, the relative ASL and all the locals of the
driver up to the moment in which optimize would be called.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the relative structures.
*/
class XPRESSModel : public AMPLModel {
  friend XPRESSDrv;

  mutable bool copied_;
  xpress::impl::XPressDriverState* state_;
  XPRSprob model_;

  XPRESSModel() :  copied_(false), state_(NULL),
    model_(NULL) {}
  
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);

public:

  XPRESSModel(const XPRESSModel& other) :
    AMPLModel(other),
    state_(other.state_),
    model_(other.model_),
    copied_(false)
  {
    fileName_ = other.fileName_;
    other.copied_ = true;
  }

  void writeSol();


  Status::Status getStatus() {
    throw AMPLSolverException("TBD");
  }

  int optimize();

  int getNumVars() {
    return getInt(XPRS_COLS);
  }
  double getObj() {
    if (getInt(XPRS_ALGORITHM) == 1) // no lp set
      return getDouble(XPRS_MIPOBJVAL);
    else
      return getDouble(XPRS_LPOBJVAL);
  }

  int getSolution(int first, int length, double* sol) {
    throw AMPLSolverException("TBD");
  }
  std::string error(int code);


  // XPRESS-specific
  // Access to XPRESS C structures
  XPRSprob getXPRSprob() {
    return model_;
  }
 
  ~XPRESSModel() {
    if (copied_)
      return;
   
  }

  int getInt(int what) {
    int ret;
    XPRSgetintattrib(model_, what, &ret);
    return ret;
  }
  int getDouble(int what) {
    double ret;
    XPRSgetdblattrib(model_, what, &ret);
    return ret;
  }
};

} // namespace
#endif // CPLEX_INTERFACE_H_INCLUDE_