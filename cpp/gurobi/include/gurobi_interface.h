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
#include <mutex>

#include "ampls/ampls.h"
#include "gurobi_callback.h"

#include "gurobi_c.h"

struct ASL;
namespace ampls
{
namespace grb
{
  namespace impl
  {
    extern "C" {
      // Imported from the GUROBI driver
      ENTRYPOINT GRBmodel* AMPLloadmodel(int argc, char** argv, ASL** asl);
      ENTRYPOINT void AMPLwritesol(GRBmodel* m, ASL* asl, int lastoptimizerun, const char* solFileName);
      ENTRYPOINT void freeEnvironment();
      ENTRYPOINT void freeASL(ASL** aslp);
    }
    // Forward declarations
    int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata);
  }
}


class GurobiModel;
class Callback;

/**
Encapsulates the main environment of the gurobi driver
*/
class GurobiDrv : public impl::SolverDriver<GurobiModel>  {
  void freeGurobiEnv();

  GurobiModel* loadModelImpl(char** args);
public:
  /**
  * Load a model from an NL file.
  * Mappings between solver row and column numbers and AMPL names are
  * available only if the row and col files have been generated as well,
  * by means of the ampl option `option auxfiles cr;` before writing the NL file.
  */
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
  friend GurobiDrv;

  mutable bool copied_;
  GRBmodel* GRBModel_;
  ASL* asl_;
  int lastErrorCode_;

  GurobiModel() : AMPLModel(), GRBModel_(NULL), asl_(NULL),
    lastErrorCode_(0), copied_(false) {}

  // Interface implementation
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);
  void writeSolImpl(const char* solFileName);
public:
  void enableLazyConstraints()
  {
    setIntParam(GRB_INT_PAR_LAZYCONSTRAINTS, 1);
  }
  GurobiModel(const GurobiModel& other) :
    AMPLModel(other), copied_(false), GRBModel_(other.GRBModel_), 
    asl_(other.asl_), lastErrorCode_(other.lastErrorCode_)
  {
    other.copied_ = true;
  }
  using AMPLModel::getSolutionVector;

  int optimize();

  Status::SolStatus getStatus() {
    int grbstatus = getIntAttr(GRB_INT_ATTR_STATUS);
    switch (grbstatus)
    {
      case GRB_LOADED:
        return Status::UNKNOWN;
      case GRB_OPTIMAL:
        return Status::OPTIMAL;
      case GRB_INFEASIBLE:
        return Status::INFEASIBLE;
      case GRB_INF_OR_UNBD:
      case GRB_UNBOUNDED:
        return Status::UNBOUNDED;
      case GRB_ITERATION_LIMIT:
        return Status::LIMIT_ITERATION;
      case GRB_NODE_LIMIT:
        return Status::LIMIT_NODE;
      case GRB_TIME_LIMIT:
        return Status::LIMIT_TIME;
      case GRB_SOLUTION_LIMIT:
        return Status::LIMIT_SOLUTION;
      case GRB_INTERRUPTED:
        return Status::INTERRUPTED;
      default:
        return Status::NOTMAPPED;
    }
  }
  int getNumVars() {
    return getIntAttr(GRB_INT_ATTR_NUMVARS);
  }
  double getObj() {
    return getDoubleAttr(GRB_DBL_ATTR_OBJVAL);
  }
  int getSolution(int first, int length, double* sol) {
    return getDoubleAttrArray(GRB_DBL_ATTR_X, first, (int)length, sol);
  }

  std::string error(int code);

  // **************** Gurobi-specific ****************

  /** Get an integer model attribute (using gurobi C library name) */
  int getIntAttr(const char* name);
  /** Get a double model attribute (using gurobi C library name) */
  double getDoubleAttr(const char* name);
  /** Get an integer array model attribute (using gurobi C library name) */
  int getIntAttrArray(const char* name, int first, int length, int* arr);
  /** Get a double array model attribute (using gurobi C library name) */
  int getDoubleAttrArray(const char* name, int first, int length, double* arr);
  /** Get an integer parameter (using gurobi C library name) */
  int getIntParam(const char* name) {
    int v;
    int error = GRBgetintparam(GRBgetenv(GRBModel_), name, &v);
    return v;
  }
  /** Get a double parameter (using gurobi C library name) */
  double getDoubleParam(const char* name) {
    double v;
    int error = GRBgetdblparam(GRBgetenv(GRBModel_), name, &v);
    return v;
  }
  /** Get a textual parameter (using gurobi C library name) */
  char* getStrParam(const char* name) {
    char* v;
    int error = GRBgetstrparam(GRBgetenv(GRBModel_), name, v);
    return v;
  }
  /** Set an integer parameter (using gurobi C library name) */
  int setIntParam(const char* name, int value) {
    return GRBsetintparam(GRBgetenv(GRBModel_), name, value);
  }
  /** Set a double parameter (using gurobi C library name) */
  double setDoubleParam(const char* name, double value) {
    return GRBsetdblparam(GRBgetenv(GRBModel_), name, value);
  }
  /** Set a textual parameter (using gurobi C library name) */
  double setStrParam(const char* name, const char* value) {
    return GRBsetstrparam(GRBgetenv(GRBModel_), name, value);
  }

  /** Get the pointer to the native C GRBmodel structure */
  GRBmodel* getGRBmodel() {
    return GRBModel_;
  }
  /** Get the pointer to the native C GRBenv structure */
  GRBenv* getGRBenv() {
    if (GRBModel_ != NULL)
      return GRBgetenv(GRBModel_);
    return NULL;
  }

  ~GurobiModel();
};
} // namespace
#endif // GUROBI_INTERFACE_H_INCLUDE_
