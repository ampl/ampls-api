#ifndef COPT_INTERFACE_H_INCLUDE_
#define COPT_INTERFACE_H_INCLUDE_

#ifdef _WIN32
  #define ENTRYPOINT __declspec(dllimport)
  #define API __declspec(dllexport)
#else
  #define ENTRYPOINT
#define API 
#endif

#include "ampls/ampls.h"
#include "copt_callback.h"
#include "copt.h"

#include <string>
#include <map>
#include <mutex>
#include <climits>  // for INT_MAX


namespace ampls
{
  namespace impl
  {
    namespace copt {
      /* Define a macro to do our error checking */
#define AMPLSCOPTERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing " #name":\n%s", error(status).c_str());

      extern "C" {
        // Imported from the copt driver library
        ENTRYPOINT void AMPLSClose_copt(void* slv);
        ENTRYPOINT void* AMPLSGetModel_copt(void* slv);
        ENTRYPOINT ampls::impl::mp::AMPLS_MP_Solver* AMPLSOpen_copt(int, char**);
      }
      // Forward declarations
      int COPT_CALL copt_callback_wrapper(copt_prob* prob, void* cbdata, int cbctx, void* userdata);
      void COPT_CALL copt_log_callback_wrapper(char* msg, void* userdata);
    } // namespace copt
  } // namespace impl

class CoptModel;
class Callback;

/**
Encapsulates the main environment of the copt driver
*/
class CoptDrv : public impl::SolverDriver<CoptModel>  {
  CoptModel loadModelImpl(char** args, const char** options);
public:
  ~CoptDrv();
};

/**
Encapsulates all the instance level information for a copt model,
namely the copt_prob object and the relative MP library.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the copt_prob and the MP structures.
*/
class CoptModel : public AMPLMPModel {
  friend CoptDrv;

  // Map for solver parameters
  std::map<int, const char*> parametersMap = {
     {SolverParams::DBL_MIPGap , COPT_DBLPARAM_RELGAP},
     {SolverParams::DBL_TimeLimit , COPT_DBLPARAM_TIMELIMIT},
     {SolverParams::INT_LP_Algorithm, COPT_INTPARAM_LPMETHOD}
  };
  const int LPalgorithmMap[4] = { -1, 0, 1, 2};
  const char* getParamAlias(SolverParams::SolverParameters params)
  {
    auto cpxParam = parametersMap.find(params);
    if (cpxParam != parametersMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  // Map for solver attributes
  std::map<int, const char*> attribsMap = {
    {SolverAttributes::DBL_CurrentObjBound, COPT_DBLATTR_BESTBND},
    {SolverAttributes::INT_NumIntegerVars, COPT_INTATTR_INTS}

  };
  const char* getAttribAlias(SolverAttributes::Attribs attrib)
  {
    auto cpxParam = attribsMap.find(attrib);
    if (cpxParam != attribsMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  copt_prob* COPTModel_;
  int lastErrorCode_;

  CoptModel() : AMPLMPModel(), COPTModel_(NULL), lastErrorCode_(0) {}
  CoptModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile, 
    const char** options) : AMPLMPModel(s, nlfile, options),
    lastErrorCode_(0) {
    COPTModel_ = (copt_prob*)impl::copt::AMPLSGetModel_copt(s);
  }
  // Interface implementation
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);

public:
  using Driver = ampls::CoptDrv;
  void enableLazyConstraints()
  {
    setParam(COPT_INTPARAM_LAZYCONSTRAINTS, 1);
  }
  CoptModel(const CoptModel& other) : AMPLMPModel(other),  
    COPTModel_(other.COPTModel_), 
    lastErrorCode_(other.lastErrorCode_) { }

  CoptModel(CoptModel&& other) noexcept:
    AMPLMPModel(std::move(other)), COPTModel_(std::move(other.COPTModel_)),
    lastErrorCode_(std::move(other.lastErrorCode_))
  {
    other.COPTModel_ = nullptr;
  }

  CoptModel& operator=(CoptModel& other) {
    if (this != &other)
    {
      AMPLMPModel::operator=(other);
      COPTModel_ = other.COPTModel_;
      lastErrorCode_ = other.lastErrorCode_;
    }
    
    return *this;
  }

  CoptModel& operator=(CoptModel&& other) noexcept {
    if (this != &other) {
      AMPLMPModel::operator=(std::move(other));
      std::swap(COPTModel_, other.COPTModel_);
      std::swap(lastErrorCode_, other.lastErrorCode_);
    }
    return *this;
  }


  
  using AMPLModel::getSolutionVector;

  const char* driver() { return "copt"; }

  Status::SolStatus getStatus() {
    if (getIntAttr(COPT_INTATTR_ISMIP)) {
      int optstatus = getIntAttr(COPT_INTATTR_MIPSTATUS);
      switch (optstatus) {
      case COPT_MIPSTATUS_OPTIMAL:
        return Status::OPTIMAL;
      case COPT_MIPSTATUS_INFEASIBLE:
        return Status::INFEASIBLE;
      case COPT_MIPSTATUS_INF_OR_UNB:
        return Status::UNBOUNDED;
      case COPT_MIPSTATUS_UNBOUNDED:
        return Status::UNBOUNDED;
      case COPT_MIPSTATUS_TIMEOUT:
      case COPT_MIPSTATUS_NODELIMIT:
      case COPT_MIPSTATUS_INTERRUPTED:
        return Status::INTERRUPTED;
      default:
        return Status::UNKNOWN;
      }
    }
    else {
      int optstatus = getIntAttr(COPT_INTATTR_LPSTATUS);
      switch (optstatus) {
      case COPT_LPSTATUS_OPTIMAL:
        return Status::OPTIMAL;
      case COPT_LPSTATUS_INFEASIBLE:
        return Status::INFEASIBLE;
      case COPT_LPSTATUS_UNBOUNDED:
        return Status::UNBOUNDED;
      case COPT_LPSTATUS_TIMEOUT:
        return Status::LIMIT_TIME;
      default:
        return Status::UNKNOWN;
      }
    }
  }
  int isMIP() {
    return getIntAttr(COPT_INTATTR_ISMIP);
  }
  int getNumVars() {
    return getIntAttr(COPT_INTATTR_COLS);
  }
  int getNumCons() {
    return getIntAttr(COPT_INTATTR_ROWS);
  };
  double getObj() {
    if (getIntAttr(COPT_INTATTR_ISMIP))
      return getDoubleAttr(COPT_DBLATTR_BESTOBJ);
    else
      return getDoubleAttr(COPT_DBLATTR_LPOBJVAL);
  }
  int getSolution(int first, int length, double* sol) {
    if ((first != 0) || (length != getNumVars()))
      throw ampls::AMPLSolverException("COPT does not support partial solution retrieval");
    if (isMIP())
      return COPT_GetSolution(COPTModel_, sol);
    else
      return COPT_GetLpSolution(COPTModel_, sol, NULL, NULL, NULL);
  }

  std::string error(int code);

  double infinity() override { return COPT_INFINITY; }


  // **************** Copt-specific ****************

  /** Get an integer model attribute (using copt C library name) */
  int getIntAttr(const char* name);
  /** Get a double model attribute (using copt C library name) */
  double getDoubleAttr(const char* name);

  /** Get an integer parameter (using copt C library name) */
  int getIntParam(const char* name) {
    int v;
    int status = COPT_GetIntParam(COPTModel_, name, &v);
    AMPLSCOPTERRORCHECK("COPT_GetIntParam")
    return v;
  }
  /** Get a double parameter (using copt C library name) */
  double getDoubleParam(const char* name) {
    double v;
    int status = COPT_GetDblParam(COPTModel_, name, &v);
    AMPLSCOPTERRORCHECK("COPT_GetDblParam")
    return v;
  }
  /** Set an integer parameter (using copt C library name) */
  void setParam(const char* name, int value) {
    int status = COPT_SetIntParam(COPTModel_, name, value);
    AMPLSCOPTERRORCHECK("COPT_SetIntParam")
  }
  /** Set a double parameter (using copt C library name) */
  void setParam(const char* name, double value) {
    int status = COPT_SetDblParam(COPTModel_, name, value);
    AMPLSCOPTERRORCHECK("COPT_SetDblParam")
  }

  /** Get the pointer to the native C COPT problem structure */
  copt_prob* getCOPTmodel() {
    return COPTModel_;
  }

  ~CoptModel();

  /**Set an integer parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    int value) {
    if (param == SolverParams::INT_LP_Algorithm)
      value = LPalgorithmMap[value];
    setParam(getParamAlias(param), value);
  }
  /**Set a double parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    double value) {    
    setParam(getParamAlias(param), value);
  }

  /**Get an integer parameter using ampls aliases*/
  int getAMPLIntParameter(SolverParams::SolverParameters params) {
    return getIntParam(getParamAlias(params));
  }
  /**Get a double parameter using ampls aliases*/
  double getAMPLDoubleParameter(SolverParams::SolverParameters params) {
    return getDoubleParam(getParamAlias(params));
  }

  /** Get an integer attribute using ampls aliases */
  int getAMPLIntAttribute(SolverAttributes::Attribs attrib) {
    return getIntAttr(getAttribAlias(attrib));
  }
  /** Get a double attribute using ampls aliases */
  double getAMPLDoubleAttribute(SolverAttributes::Attribs attrib) {
    switch (attrib)
    {
    case SolverAttributes::DBL_RelMIPGap:
      return impl::calculateRelMIPGAP(getObj(),
        getAMPLDoubleAttribute(SolverAttributes::DBL_CurrentObjBound));
    default:
      return getDoubleAttr(getAttribAlias(attrib));
    }
  }

  std::vector<double> getConstraintsValueImpl(const std::vector<int>& indices) {
    std::vector<double> values(getNumVars());
    COPT_GetLpSolution(COPTModel_, NULL, NULL, NULL, values.data());
    std::vector<double> res(indices.size());
    for (int i = 0; i < indices.size(); i++)
      res[i] = values[indices[i]];
    return res;
  }

  int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {
    char coptsense = CoptCallback::toCOPTSense(sense);
    int status = COPT_AddRow(COPTModel_, numnz, const_cast<int*>(vars),
      const_cast<double*>(coefficients), coptsense, rhs, 0, name);
    return getNumCons()-1;
  }

  int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {
    char t;
    switch (type)
    {
      case VarType::Continuous:
        t = COPT_CONTINUOUS;
        break;
      case VarType::Binary:
        t = COPT_BINARY;
        break;
      case VarType::Integer:
        t = COPT_INTEGER;
        break;
    }
    int status = 0;
    if (numnz == 0)
    {
       status = COPT_AddCol(COPTModel_, objcoeff, 0,NULL, NULL, t, lb, ub, name);
    }
    else {
      status = COPT_AddCol(COPTModel_, objcoeff, numnz, const_cast<int*>(cons),
        const_cast<double*>(coefficients), t, lb, ub,  name);
    }
    if (status != 0)
      throw ampls::AMPLSolverException::format("Could not add variable, code: %d.", status);;
    return getNumVars()-1;
  }

  std::vector<double>  getConstraintsValueImpl(int offset, int length)
  {
    std::vector<double> pi(getNumCons());

    COPT_GetLpSolution(COPTModel_, NULL, NULL, pi.data(), NULL);
    auto first = pi.begin() + offset;
    auto last = pi.end();
    return std::vector<double>(first, last);
  }
  std::vector<double> getVarsValueImpl(int offset, int length) {
    std::vector<double> values(getNumVars());
    COPT_GetLpSolution(COPTModel_,values.data(), NULL, NULL, NULL);
    auto first = values.begin() + offset;
    auto last = values.end();
    return std::vector<double>(first, last);
  }


};
} // namespace
#endif // COPT_INTERFACE_H_INCLUDE_
