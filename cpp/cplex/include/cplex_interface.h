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





namespace ampls
{

class CPLEXCallback;
class CPLEXModel;


  namespace impl
  {
    namespace cpx
    {
      
  /* Define a macro to do our error checking */
    #define AMPLSCPXERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing " #name":\n%s", error(status).c_str());

    extern "C" {
      // Imported from the gurobi driver library
      ENTRYPOINT void AMPLSClose_cplexmp(void* slv);
      ENTRYPOINT void* AMPLSGetModel_cplexmp(void* slv);
      ENTRYPOINT void* AMPLSGetEnv_cplexmp(void* slv);
      ENTRYPOINT void* AMPLSOpen_cplexmp(int, char**);
    }
    
    class CBWrap {
      
    public:
      static bool skipMsgCallback;
      static int CPXPUBLIC genericcallback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
        void* cbhandle);
      static void CPXPUBLIC msg_callback_wrapper(void* handle, const char* msg);
    };
  
  }
}



/**
Encapsulates the main environment of the CPLEX driver;
without modifications, a static CPLEXENV is created in the
AMPL driver, and it would be fairly easy to lose track of it;
this way, it is deleted in the destructor. 
*/
class CPLEXDrv : public impl::SolverDriver<CPLEXModel> {
  void freeCPLEXEnv();
  CPLEXModel loadModelImpl(char** args, const char** options);
public:
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
class CPLEXModel : public AMPLMPModel {
  friend CPLEXDrv;

  // Map from ampls solverparams to CPLEX parameters
  std::map<int, int> parametersMap = {
     {SolverParams::INT_SolutionLimit , CPXPARAM_MIP_Limits_Solutions},
     {SolverParams::DBL_MIPGap , CPXPARAM_MIP_Tolerances_MIPGap},
     {SolverParams::DBL_TimeLimit , CPXPARAM_TimeLimit},
     {SolverParams::INT_LP_Algorithm, CPXPARAM_LPMethod}
  };
  const int LPalgorithmMap[4] = { CPX_ALG_AUTOMATIC, CPX_ALG_PRIMAL, CPX_ALG_DUAL, CPX_ALG_BARRIER };

  int getCPXParamAlias(SolverParams::SolverParameters params)
  {
    auto cpxParam = parametersMap.find(params);
    if (cpxParam != parametersMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  int status_;
  CPXLPptr model_;
  int lastErrorCode_;

  CPLEXModel() : AMPLMPModel(), model_(nullptr), lastErrorCode_(0) {}

  CPLEXModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile,
    const char** options) : AMPLMPModel(s, nlfile, options),
    lastErrorCode_(0) {
    model_ = (CPXLPptr)impl::cpx::AMPLSGetModel_cplexmp(s);
  }


  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);


 // std::vector<double> getConstraintsValueImpl(int offset, int length);
 // std::vector<double> getVarsValueImpl(int offset, int length);

public:
  using Driver = ampls::CPLEXDrv;

  CPLEXModel(const CPLEXModel& other) : AMPLMPModel(other),
    status_(other.status_),
    model_(other.model_),
    lastErrorCode_(other.lastErrorCode_) { }

  CPLEXModel(CPLEXModel&& other) noexcept :
    AMPLMPModel(std::move(other)), status_(std::move(other.status_)),
    model_(std::move(other.model_)), lastErrorCode_(std::move(other.lastErrorCode_))
  {
    other.model_ = nullptr;
  }
  CPLEXModel& operator=(CPLEXModel& other) {
    if (this != &other)
    {
      AMPLMPModel::operator=(other);
      status_ = other.status_;
      model_ = other.model_;
      lastErrorCode_ = other.lastErrorCode_;
    }

    return *this;
  }

  CPLEXModel& operator=(CPLEXModel&& other) noexcept {
    if (this != &other) {
      AMPLMPModel::operator=(std::move(other));
      std::swap(status_, other.status_);
      std::swap(model_, other.model_);
      std::swap(lastErrorCode_, other.lastErrorCode_);
    }
    return *this;
  }

  const char* driver() { return "cplex"; }

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
    case CPX_STAT_CONFLICT_MINIMAL:
    case CPX_STAT_MULTIOBJ_INFEASIBLE:
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
    case CPX_STAT_CONFLICT_ABORT_TIME_LIM:
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

  void optimize();

  int getNumVars() {
    return CPXXgetnumcols(getCPXENV(), model_);
  }

  int getNumCons() {
    return CPXXgetnumrows(getCPXENV(), model_);
  };

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

  double infinity() override { return CPX_INFBOUND; }

  // ********************* CPLEX specific *********************
  int setCallback(impl::BaseCallback* callback) {
    int i = ampls::AMPLModel::setCallback(callback);
    return i;
  }
  int setCallback(GenericCallback* callback) {
    int i = ampls::AMPLModel::setCallback(callback);
    return i;
  }
  /** Get the pointer to the native CPLEX LP model object */
  CPXLPptr getCPXLP() {
    return model_;
  }
  /** Get the pointer to the native CPLEX environment object */
  CPXENVptr getCPXENV() {
    return (CPXENVptr)impl::cpx::AMPLSGetEnv_cplexmp(solver_);
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

  /** Get an integer CPLEX control parameter */
  int getIntParam(int CPXPARAM) {
    int value;
    int status = CPXgetintparam(getCPXENV(), CPXPARAM, &value);
    AMPLSCPXERRORCHECK("CPXgetintparam");
    return value;
  }

  /** Get a double CPLEX control parameter */
  double getDoubleParam(int CPXPARAM) {
    double value;
    int status = CPXgetdblparam(getCPXENV(), CPXPARAM, &value);
    AMPLSCPXERRORCHECK("CPXgetdblparam");
    return value;
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
  }

  /**Get an integer parameter using ampls aliases*/
  int getAMPLIntParameter(SolverParams::SolverParameters param) {
    return getIntParam(getCPXParamAlias(param));
  }
  /**Get a double parameter using ampls aliases*/
  double getAMPLDoubleParameter(SolverParams::SolverParameters param) {
    return getDoubleParam(getCPXParamAlias(param));
  }

  /**Set an integer parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    int value) {
    if (param == SolverParams::INT_LP_Algorithm)
      value = LPalgorithmMap[value];
    setParam(getCPXParamAlias(param), value);
  }
  /**Set a double parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    double value) {
    setParam(getCPXParamAlias(param), value);
  }



  /** Get an integer attribute using ampls aliases */
  int getAMPLIntAttribute(SolverAttributes::Attribs attrib) {
    int val;
    int status;
    switch (attrib)
    {
    case SolverAttributes::INT_NumIntegerVars:
      return CPXXgetnumint(getCPXENV(), getCPXLP());
    }
    throw ampls::AMPLSolverException("Not supported");
  }
  /** Get a double attribute using ampls aliases */
  double getAMPLDoubleAttribute(SolverAttributes::Attribs attrib) {
    double val;
    int status;
    switch (attrib)
    {
    case SolverAttributes::DBL_RelMIPGap:
      double obj;
      CPXgetbestobjval(getCPXENV(), getCPXLP(), &val);
      CPXgetmipobjval(getCPXENV(), getCPXLP(), &obj);
      return impl::calculateRelMIPGAP(obj, val);
    case SolverAttributes::DBL_CurrentObjBound:
      status = CPXgetbestobjval(getCPXENV(), getCPXLP(), &val);
      AMPLSCPXERRORCHECK("CPXgetbestobjval")
      break;
    default:
      throw ampls::AMPLSolverException("Not supported");
    }
    
    return val;
  }

  int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {
    double rhsd[] = { rhs };
    char sensed[] = { CPLEXCallback::toCPLEXSense(sense) };
    int rowbegin[] = { 0 };
    char* named[] = { const_cast<char*>(name) };

    // TODO Infinity CPX_INFBOUND 
    int status = CPXaddrows(getCPXENV(), getCPXLP(), 0, 1,
      numnz, rhsd, sensed, rowbegin, vars, coefficients, NULL, named);
    AMPLSCPXERRORCHECK("CPXaddrows")
    return getNumCons() - 1;
  }
  const char toCPLEXType[3] = { CPX_CONTINUOUS, CPX_BINARY, CPX_INTEGER };
  int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {
    char* named[] = { const_cast<char*>(name) };
    int status= CPXnewcols(getCPXENV(), getCPXLP(), 1, &objcoeff, &lb, &ub, NULL, named);
    AMPLSCPXERRORCHECK("CPXnewcols");
    int varindex= getNumVars() - 1;
    std::vector<int> colindex(numnz, varindex - 1);
    status = CPXchgcoeflist(getCPXENV(), getCPXLP(), numnz, cons, colindex.data(), coefficients);
    AMPLSCPXERRORCHECK("CPXaddcols");

    if (type != ampls::VarType::Continuous) {
      char varType = toCPLEXType[(int)type];
      CPXchgctype(getCPXENV(), getCPXLP(), 1, &varindex, &varType);
    }
    return varindex;
  }

  std::vector<double>  getConstraintsValueImpl(int offset, int length);
  std::vector<double> getVarsValueImpl(int offset, int length);

};

} // namespace
#endif // CPLEX_INTERFACE_H_INCLUDE_