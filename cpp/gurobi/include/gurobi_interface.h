#ifndef GUROBI_INTERFACE_H_INCLUDE_
#define GUROBI_INTERFACE_H_INCLUDE_

#ifdef _WIN32
  #define ENTRYPOINT __declspec(dllimport)
  #define API __declspec(dllexport)
#else
  #define ENTRYPOINT
#define API 
#endif


#include <string>
#include <map>

//#include "simpleapi/simpleApi.h"
#include "gurobi_callback.h"

#include "gurobi_c.h"

struct ASL;
namespace ampl
{
namespace grb
{
  namespace impl
  {
    extern "C" {
      // Imported from the GUROBI driver
      ENTRYPOINT GRBmodel* AMPLloadmodel(int argc, char** argv);
      ENTRYPOINT GRBmodel* AMPLloadmodelNoLic(int argc, char** argv, ASL** asl);
      ENTRYPOINT void AMPLwritesol(GRBmodel* m, ASL* asl, int lastoptimizerun);
      ENTRYPOINT void freeEnvironment();
      ENTRYPOINT void freeASL(ASL** aslp);
    }
  }
}

// Forward declarations
int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata);
class GurobiModel;
class Callback;

/**
Encapsulates the main environment of the gurobi driver;
without modifications, a static GRBenv is created in the
AMPL driver, and it would be fairly easy to lose track of it;
this way, it is deleted in the destructor.
*/
class GurobiDrv {
  void freeGurobiEnv();
public:
  GurobiModel loadModel(const char* modelName);
  ~GurobiDrv();

};

/**
Encapsulates all the instance level information for a gurobi model,
namely the GRBmodel object and the relative ASL.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the GRBmodel and the ASL structures.
Note that if we don't want to use the writesol function, we don't really
need the ASL reference after creating the Gurobi model object, so
we could use directly the C pointer to GRBmodel.
*/
class GurobiModel : public AMPLModel {
  friend GurobiModel GurobiDrv::loadModel(const char* modelName);

  mutable bool copied_;
  GRBmodel* GRBModel_;
  ASL* asl_;
  int lastErrorCode_;



  GurobiModel() : AMPLModel(), GRBModel_(NULL), asl_(NULL),
    lastErrorCode_(0), copied_(false) {}




public:
  GurobiModel(const GurobiModel& other) :
    AMPLModel(other)
  {
    copied_ = false;
    asl_ = other.asl_;
    GRBModel_ = other.GRBModel_;
    lastErrorCode_ = other.lastErrorCode_;
    other.copied_ = true;
  }
  using AMPLModel::getSolutionVector;

  // Interface implementation
  void writeSol();

  int optimize();
  int getNumVars() {
    return getIntAttr(GRB_INT_ATTR_NUMVARS);
  }
  double getObj() {
    return getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  }
  int getSolution(int first, int length, double* sol) {
    return getDoubleAttrArray(GRB_DBL_ATTR_X, first, (int)length, sol);
  }
  int setCallbackDerived(BaseCallback* callback);
  BaseCallback* createCallbackImplDerived(GenericCallback* callback);
  std::string error(int code);

  // Gurobi-specific
  int getIntAttr(const char* name);
  double getDoubleAttr(const char* name);

  int getIntAttrArray(const char* name, int first, int length, int* arr);
  int getDoubleAttrArray(const char* name, int first, int length, double* arr);

  int getIntParam(const char* name) {
    int v;
    int error = GRBgetintparam(GRBgetenv(GRBModel_), name, &v);
    return v;
  }
  double getDoubleParam(const char* name) {
    double v;
    int error = GRBgetdblparam(GRBgetenv(GRBModel_), name, &v);
    return v;
  }
  char* getStrParam(const char* name) {
    char* v;
    int error = GRBgetstrparam(GRBgetenv(GRBModel_), name, v);
    return v;
  }

  int setIntParam(const char* name, int value) {
    return GRBsetintparam(GRBgetenv(GRBModel_), name, value);
  }
  double setDoubleParam(const char* name, double value) {
    return GRBsetdblparam(GRBgetenv(GRBModel_), name, value);
  }
  double setStrParam(const char* name, const char* value) {
    return GRBsetstrparam(GRBgetenv(GRBModel_), name, value);
  }

  // Access to gurobi C structures
  GRBmodel* getGRBmodel() {
    return GRBModel_;
  }
  GRBenv* getGRBenv() {
    if (GRBModel_ != NULL)
      return GRBgetenv(GRBModel_);
    return NULL;
  }

  ~GurobiModel();

};
} // namespace
#endif // GUROBI_INTERFACE_H_INCLUDE_
