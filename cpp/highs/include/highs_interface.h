#ifndef HIGHS_INTERFACE_H_INCLUDE_
#define HIGHS_INTERFACE_H_INCLUDE_

#ifdef _WIN32
  #define ENTRYPOINT __declspec(dllimport)
  #define API __declspec(dllexport)
#else
  #define ENTRYPOINT
#define API 
#endif

#include "ampls/ampls.h"
#include "highs_callback.h"
#include "interfaces/highs_c_api.h"

namespace ampls
{
  namespace impl{
    namespace highs {

    #define AMPLSHIGHSERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing " #name":\n%s", error(status).c_str());

      extern "C" {
        // Imported from the highs driver library
        ENTRYPOINT void AMPLSClose_highs(void* slv);
        ENTRYPOINT void* AMPLSGetModel_highs(void* slv);
        ENTRYPOINT ampls::impl::mp::AMPLS_MP_Solver* AMPLSOpen_highs(int, char**);
      }

      static void highs_callback_wrapper(const int where,
        const char* message, const HighsCallbackDataOut* dataout,
        HighsCallbackDataIn* datain, void* userdata);
    } // namespace highs
  } // namespace impl

class HighsModel;
class Callback;

/**
Encapsulates the main environment of the highs driver
*/
class HighsDrv : public impl::SolverDriver<HighsModel>  {
  HighsModel loadModelImpl(char** args, const char** options);
public:
  ~HighsDrv();
};

/**
Encapsulates all the instance level information for a highs model,
namely the highs_prob object and the relative MP library.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the highs_prob and the MP structures.
*/
class HighsModel : public AMPLMPModel {
  friend HighsDrv;

  // Map for solver parameters
  
  std::map<int, const char*> parametersMap = {
    // TODO
     {SolverParams::DBL_MIPGap , "mip_rel_gap"},
     {SolverParams::DBL_TimeLimit , "time_limit"},
     {SolverParams::INT_LP_Algorithm, "solver"}
  };
  const int LPalgorithmMap[4] = { -1, 1, 1, 2};
  const char* getParamAlias(SolverParams::SolverParameters params)
  {
    auto highsParam = parametersMap.find(params);
    if (highsParam != parametersMap.end())
      return highsParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  // Map for solver attributes
  std::map<int, const char*> attribsMap = {
    {SolverAttributes::DBL_CurrentObjBound, "HIGHS_DBLATTR_BESTBND"},
    {SolverAttributes::INT_NumIntegerVars, "HIGHS_INTATTR_INTS"}

  };
  const char* getAttribAlias(SolverAttributes::Attribs attrib)
  {
    auto cpxParam = attribsMap.find(attrib);
    if (cpxParam != attribsMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  void* HighsModel_;
  int lastErrorCode_;

  HighsModel() : AMPLMPModel(), HighsModel_(NULL), lastErrorCode_(0) {}
  HighsModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile, 
    const char** options) : AMPLMPModel(s, nlfile, options),
    lastErrorCode_(0) {
    HighsModel_ = impl::highs::AMPLSGetModel_highs(s);
  }
  // Interface implementation
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);

public:
  using Driver = ampls::HighsDrv;
  void enableLazyConstraints()
  {
    throw ampls::AMPLSolverException("Not supported by HiGHS yet");
  }
  HighsModel(const HighsModel& other) : AMPLMPModel(other),  
    HighsModel_(other.HighsModel_), 
    lastErrorCode_(other.lastErrorCode_) { }

  HighsModel(HighsModel&& other) noexcept:
    AMPLMPModel(std::move(other)), HighsModel_(std::move(other.HighsModel_)),
    lastErrorCode_(std::move(other.lastErrorCode_))
  {
    other.HighsModel_ = nullptr;
  }

  HighsModel& operator=(HighsModel& other) {
    if (this != &other)
    {
      AMPLMPModel::operator=(other);
      HighsModel_ = other.HighsModel_;
      lastErrorCode_ = other.lastErrorCode_;
    }
    
    return *this;
  }

  HighsModel& operator=(HighsModel&& other) noexcept {
    if (this != &other) {
      AMPLMPModel::operator=(std::move(other));
      std::swap(HighsModel_, other.HighsModel_);
      std::swap(lastErrorCode_, other.lastErrorCode_);
    }
    return *this;
  }
  using AMPLModel::getSolutionVector;

  const char* driver() { return "highs"; }

  Status::SolStatus getStatus() {
    int optstatus = Highs_getModelStatus(HighsModel_);
    switch (optstatus) {
    case kHighsModelStatusOptimal:
      return Status::OPTIMAL;
    case kHighsModelStatusInfeasible:
      return Status::INFEASIBLE;
    case kHighsModelStatusUnbounded:
        return Status::UNBOUNDED;
    case kHighsModelStatusUnboundedOrInfeasible:
      return Status::INFEASIBLE;
    case kHighsModelStatusModelError:
    case kHighsModelStatusLoadError:
      return Status::UNKNOWN;
    case kHighsModelStatusPresolveError:
    case kHighsModelStatusSolveError:
    case kHighsModelStatusPostsolveError:
      return Status::UNKNOWN;
    case kHighsModelStatusTimeLimit:
      return Status::LIMIT_TIME;
    case kHighsModelStatusIterationLimit:
      return Status::LIMIT_ITERATION;
    case kHighsModelStatusSolutionLimit:
      return Status::LIMIT_SOLUTION;
    case kHighsModelStatusInterrupt:
      return Status::INTERRUPTED;
    case kHighsModelStatusObjectiveBound:
    case kHighsModelStatusObjectiveTarget:
      return Status::NOTMAPPED;
    default:
      return Status::UNKNOWN;
    }
  }
  int isMIP() {
    // Very expensive on HiGHS 1.7
    int integrality;
    for (int i = 0; i < getNumVars(); ++i) {
      Highs_getColIntegrality(HighsModel_, i, &integrality);
      if (integrality >= kHighsVarTypeContinuous)
        return true;
    }
    return false;
  }
  int getNumVars() {
    return Highs_getNumCol(HighsModel_);
  }
  int getNumCons() {
    return Highs_getNumRow(HighsModel_);
  };
  double getObj() {
    return Highs_getObjectiveValue(HighsModel_);
  }

  int getSolution(int first, int length, double* sol) {

    if ((first != 0) && (length != getNumVars()))
      throw ampls::AMPLSolverException("HiGHS does not support partial solution extraction");
    Highs_getSolution(HighsModel_, sol, NULL, NULL, NULL);
    return 0;
  }

  std::string error(int code);

  double infinity() override { return Highs_getInfinity(HighsModel_); }


  // **************** Highs-specific ****************

  /** Get an integer model attribute (using highs C library name) */
  int getIntAttr(const char* name);
  /** Get an integer model attribute (using highs C library name) */
  int64_t getInt64Attr(const char* name);
  /** Get a double model attribute (using highs C library name) */
  double getDoubleAttr(const char* name);

  /** Get a boolean parameter (using highs C library name) */
  bool getBoolParam(const char* name) {
    int v;
    int status = Highs_getBoolOptionValue(HighsModel_, name, &v);
    AMPLSHIGHSERRORCHECK("Highs_getBoolOptionValue")
      return v;
  }

  /** Get an integer parameter (using highs C library name) */
  int getIntParam(const char* name) {
    int v;
    int status = Highs_getIntOptionValue(HighsModel_, name, &v);
    AMPLSHIGHSERRORCHECK("Highs_getIntOptionValue")
    return v;
  }
  /** Get a double parameter (using highs C library name) */
  double getDoubleParam(const char* name) {
    double v;
    int status = Highs_getDoubleOptionValue(HighsModel_, name, &v);
    AMPLSHIGHSERRORCHECK("Highs_getDoubleOptionValue")
  }
  /** Get a string parameter (using highs C library name) */
  std::string getStringParam(const char* name) {
    char BUFFER[kHighsMaximumStringLength];
    double v;
    int status = Highs_getStringOptionValue(HighsModel_, name, BUFFER);
    AMPLSHIGHSERRORCHECK("Highs_getStringOptionValue");
    return std::string(BUFFER);
  }


  /** Set a boolean parameter (using highs C library name) */
  void setParam(const char* name, bool value) {
    int status = Highs_setBoolOptionValue(HighsModel_, name, value);
    AMPLSHIGHSERRORCHECK("Highs_setBoolOptionValue")
  }

  /** Set an integer parameter (using highs C library name) */
  void setParam(const char* name, int value) {
    int status = Highs_setIntOptionValue(HighsModel_, name, value);
    AMPLSHIGHSERRORCHECK("Highs_setIntOptionValue")
  }
  /** Set a double parameter (using highs C library name) */
  void setParam(const char* name, double value) {
    int status = Highs_setDoubleOptionValue(HighsModel_, name, value);
    AMPLSHIGHSERRORCHECK("HIGHS_SetDblParam")
  }
  /** Set a stringparameter (using highs C library name) */
  void setParam(const char* name, const char* value) {
    int status = Highs_setStringOptionValue(HighsModel_, name, value);
    AMPLSHIGHSERRORCHECK("Highs_setStringOptionValue")
  }

  /** Get the pointer to the native C HIGHS problem structure */
  void* getHighsModel() {
    return HighsModel_;
  }

  ~HighsModel();

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



  int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {

    double alhs = rhs, arhs = rhs;
    if (sense == CutDirection::GE)
        arhs = infinity();
    if (sense == CutDirection::LE)
        alhs = -infinity();
    Highs_addRow(HighsModel_, alhs, arhs, numnz, vars, coefficients);
    return getNumCons()-1;
  }

  int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {
    char t;
    Highs_addCol(HighsModel_, objcoeff, lb, ub,
      numnz, cons, coefficients);
    int v = getNumVars() - 1;
    switch (type)
    {
      case VarType::Integer:
      case VarType::Binary:
        Highs_changeColIntegrality(HighsModel_, v, kHighsVarTypeInteger);
    }
    return v;
  }

  std::vector<double>  getConstraintsValueImpl(int offset, int length)
  {
    std::vector<double> pi(getNumCons());
    int error = Highs_getSolution(HighsModel_, NULL, NULL, NULL, pi.data());
    auto first = pi.begin() + offset;
    auto last = pi.end();
    return std::vector<double>(first, last);
  }
  std::vector<double> getVarsValueImpl(int offset, int length) {
    std::vector<double> values(getNumVars());
    Highs_getSolution(HighsModel_, values.data(), NULL, NULL, NULL);
    auto first = values.begin() + offset;
    auto last = values.end();
    return std::vector<double>(first, last);
  }


};
} // namespace
#endif // HIGHS_INTERFACE_H_INCLUDE_
