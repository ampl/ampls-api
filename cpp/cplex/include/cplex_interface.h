#ifndef CPLEX_INTERFACE_H_INCLUDE_
#define CPLEX_INTERFACE_H_INCLUDE_

#include <string>
#include <map>
#include <mutex>

#if CPX_APIMODEL == CPX_APIMODEL_LARGE
#include "ilcplex/cplexx.h"
#endif
#include "ilcplex/cplex.h"

#include "ampls/ampls.h"
#include "cplex_callback.h"

#ifdef _WIN32
#define CPXPUBLIC      __stdcall
#define CPXPUBVARARGS  __cdecl
#define CPXCDECL       __cdecl
#else
#define CPXPUBLIC
#define CPXPUBVARARGS
#define CPXCDECL
#endif

struct ASL;


namespace ampls
{

class CPLEXCallback;
class CPLEXModel;

namespace cpx
{
  namespace impl
  {
  /* Define a macro to do our error checking */
    #define AMPLSCPXERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing #name: %s", error(status).c_str());

    struct CPLEXDriverState;
    extern "C" {

      ENTRYPOINT CPLEXDriverState* AMPLCPLEXloadmodel(int argc, char** argv,
        CPXLPptr* modelPtr, ASL** aslPtr);

      ENTRYPOINT void AMPLCPLEXwritesol(CPLEXDriverState* state,
        CPXLPptr modelPtr, int status, const char* solFileName);

      ENTRYPOINT CPXENVptr AMPLCPLEXgetInternalEnv();

      ENTRYPOINT void AMPLCPLEXfreeASL(ASL** aslPtr);
    }
    
    class CBWrap {
    public:
      static int CPXPUBLIC lp_callback_wrapper(CPXCENVptr env, void* lp, int wf, void* cbh);
      static int CPXPUBLIC cut_callback_wrapper(CPXCENVptr env, void* cbdata, int wherefrom,
        void* cbhandle, int* useraction_p);
      static void CPXPUBLIC msg_callback_wrapper(void* handle, const char* msg);
      static int CPXPUBLIC incumbent_callback_wrapper(CPXCENVptr env,
        void* cbdata, int wherefrom, void* cbhandle, double objval, double* x, int* isfeas_p,
        int* useraction_p);
      static CPLEXCallback* setDefaultCB(CPXCENVptr env, void* cbdata,
        int wherefrom, void* userhandle);
    };
  
  }
}



/**
Encapsulates the main environment of the gurobi driver;
without modifications, a static CPLEXENV is created in the
AMPL driver, and it would be fairly easy to lose track of it;
this way, it is deleted in the destructor. 
*/
class CPLEXDrv : public impl::SolverDriver<CPLEXModel> {
  void freeCPLEXEnv();
  CPLEXModel* loadModelImpl(char** args);
public:
 /**
 * Load a model from an NL file.
 * Mappings between solver row and column numbers and AMPL names are
 * available only if the row and col files have been generated as well,
 * by means of the ampl option `option auxfiles cr;` before writing the NL file.
 */
  CPLEXModel loadModel(const char* modelName);
  /**
   * Get the pointer to the native `CPXENVptr` wrapped by this driver
  */
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
  friend CPLEXDrv;

  mutable bool copied_;
  cpx::impl::CPLEXDriverState* state_;
  int status_;
  CPXLPptr model_;
  ASL* asl_;
  int lastErrorCode_;
  CPLEXModel() :  copied_(false), state_(NULL), status_(0) , 
    model_(NULL), asl_(NULL),lastErrorCode_(0) {}
  
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);
  void writeSolImpl(const char* solFileName);

public:
  CPLEXModel(const CPLEXModel& other) :
    AMPLModel(other),
    state_(other.state_),
    asl_(other.asl_),
    model_(other.model_),
    lastErrorCode_(other.lastErrorCode_),
    copied_(false),
    status_(other.status_)
  {
    fileName_ = other.fileName_;
    other.copied_ = true;
  }
  
  Status::SolStatus getStatus() {
    int cpxstatus = CPXgetstat(getCPXENV(), model_);
    switch (cpxstatus) 
    {
      case CPX_STAT_OPTIMAL: // simplex and barrier optimal
      case CPXMIP_OPTIMAL: // MIP optimal
      case CPXMIP_OPTIMAL_TOL:
        return Status::OPTIMAL;
      case CPX_STAT_INFEASIBLE: // Problem infeasible
      case CPXMIP_INFEASIBLE:
        return Status::INFEASIBLE;
      case CPX_STAT_UNBOUNDED: // Problem unbounded
      case CPXMIP_UNBOUNDED:
        return Status::UNBOUNDED;
      case CPX_STAT_ABORT_OBJ_LIM: // Objective limit exceeded
      case CPX_STAT_ABORT_PRIM_OBJ_LIM: 
      case CPX_STAT_ABORT_DUAL_OBJ_LIM:
      case CPX_STAT_CONFLICT_ABORT_OBJ_LIM:
      case CPXMIP_SOL_LIM:
        return Status::LIMIT_SOLUTION;
      case CPX_STAT_ABORT_IT_LIM:
      case CPX_STAT_CONFLICT_ABORT_IT_LIM:
      case CPXMIP_NODE_LIM_FEAS:
      case CPXMIP_NODE_LIM_INFEAS:
        return Status::LIMIT_ITERATION;
      case CPX_STAT_ABORT_DETTIME_LIM:
      case CPX_STAT_ABORT_TIME_LIM:
      case CPX_STAT_CONFLICT_ABORT_DETTIME_LIM:
      case CPXMIP_DETTIME_LIM_FEAS:
      case CPXMIP_DETTIME_LIM_INFEAS:
      case CPXMIP_TIME_LIM_FEAS:
      case CPXMIP_TIME_LIM_INFEAS:
        return Status::LIMIT_TIME;
      case CPX_STAT_ABORT_USER:
      case CPXMIP_ABORT_FEAS:
      case CPXMIP_ABORT_INFEAS:
      case CPXMIP_ABORT_RELAXATION_UNBOUNDED:
      case CPXMIP_ABORT_RELAXED:
        return Status::INTERRUPTED;
      default:
        return Status::UNKNOWN;
    }
  }

  int optimize();

  int getNumVars() {
    return CPXXgetnumcols(getCPXENV(), model_);
  }
  double getObj() {
    double obj;
    int status = CPXgetobjval(getCPXENV(), model_, &obj);
    AMPLSCPXERRORCHECK("CPXgetobjval");
    return obj;
  }

  int getSolution(int first, int length, double* sol) {
    int status = CPXgetx(getCPXENV(), model_, sol, first, length - 1);
    AMPLSCPXERRORCHECK("CPXgetx");
    return status;
  }
  std::string error(int code);

  void enableLazyConstraints() {
    setParam(CPXPARAM_MIP_Strategy_CallbackReducedLP, CPX_OFF);
    setParam(CPXPARAM_Preprocessing_Linear, 0);
  }
  
  // ********************* CPLEX specific *********************

  /** Get the pointer to the native CPLEX LP model object */
  CPXLPptr getCPXLP() {
    return model_;
  }
  /** Get the pointer to the native CPLEX environment object */
  CPXENVptr getCPXENV() {
    return cpx::impl::AMPLCPLEXgetInternalEnv();
  }
  /** Set an integer CPLEX control parameter */
  void setParam(int CPXPARAM, int value) {
    int status = CPXsetintparam(getCPXENV(), CPXPARAM, value);
    AMPLSCPXERRORCHECK("CPXsetintparam");
  }
  /** Set a double CPLEX control parameter */
  void setParam(int CPXPARAM, double value) {
    int status = CPXsetdblparam(getCPXENV(), CPXPARAM, value);
    AMPLSCPXERRORCHECK("CPXsetdblparam");
  }

  ~CPLEXModel() {
    // TODO: This way the CPLEX environment is closed when the model is closed, wich makes sense
    // because in this implemenatation a new environment is created when loading a new model.
    // It would be then desirable to incorporate CPLEXDriver into CPLEXModel.
    if (copied_)
      return;
    if (model_)
      CPXfreeprob(getCPXENV(), &model_);
    CPXENVptr env = getCPXENV();
    CPXcloseCPLEX(&env);
    if (asl_)
      cpx::impl::AMPLCPLEXfreeASL(&asl_);
  }
};

} // namespace
#endif // CPLEX_INTERFACE_H_INCLUDE_