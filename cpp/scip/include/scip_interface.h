#ifndef SCIP_INTERFACE_H_INCLUDE_
#define SCIP_INTERFACE_H_INCLUDE_

#ifdef _WIN32
  #define ENTRYPOINT __declspec(dllimport)
  #define API __declspec(dllexport)
#else
  #define ENTRYPOINT
#define API 
#endif

#include "ampls/ampls.h"
#include "scip_callback.h"

#include <string>
#include <map>
#include <mutex>
#include <climits>  // for INT_MAX

/// problem data stored in SCIP
struct SCIP_ProbData
{
   SCIP_VAR**            vars;               /**< variables in the order given by AMPL */
   int                   nvars;              /**< number of variables */

   SCIP_CONS**           linconss;           /**< linear constraints in the order given by AMPL */
   int                   i;                  /**< shows free slot of linear constraints */
   int                   nlinconss;          /**< number of linear constraints */
};


namespace ampls
{

  namespace impl
  {
    namespace scip
    {
    /* Define a macro to do our error checking */
    #define AMPLSSCIPERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing " #name":\n%s", error(status).c_str());

    extern "C" {
      ENTRYPOINT void AMPLSClose_scip(void* slv);
      ENTRYPOINT void* AMPLSGetModel_scip(void* slv);
      ENTRYPOINT void* AMPLSOpen_scip(int, char**);
    }
  }
}


class SCIPModel;
class Callback;

/**
Encapsulates the main environment of the SCIP driver
*/
class SCIPDrv : public impl::SolverDriver<SCIPModel>  {
  SCIPModel loadModelImpl(char** args, const char** options);
public:
  ~SCIPDrv();
};

/**
Encapsulates all the instance level information for a SCIP model,
namely the SCIP object and the relative MP library.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the SCIP and the MP structures.
*/
class SCIPModel : public AMPLMPModel {
  friend SCIPDrv;

  // Map for solver parameters
  std::map<int, const char*> parametersMap = {
     {SolverParams::INT_SolutionLimit , "limits/solutions"},
     {SolverParams::DBL_MIPGap , "limits/gap"},
     {SolverParams::DBL_TimeLimit , "limits/time"},
     {SolverParams::INT_LP_Algorithm, "lp/initalgorithm"}
  };
  const int LPalgorithmMap[4] = { -1, 0, 1, 2};
  const char* getSCIPParamAlias(SolverParams::SolverParameters params)
  {
    auto cpxParam = parametersMap.find(params);
    if (cpxParam != parametersMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  SCIP* model_;

  SCIPModel() : AMPLMPModel(), model_(NULL) {}

  SCIPModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile, 
    const char** opt) : AMPLMPModel(s, nlfile, opt) {
    model_ = (SCIP*) impl::scip::AMPLSGetModel_scip(s);
  }

  // Interface implementation
  int setCallbackDerived(impl::BaseCallback* callback);
  //impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);
  void writeSolImpl(const char* solFileName);
public:
  using Driver = ampls::SCIPDrv;

  void enableLazyConstraints()
  {
    
  }
  SCIPModel(const SCIPModel& other) :
    AMPLMPModel(other), model_(other.model_) { }

  SCIPModel(SCIPModel&& other) noexcept :
    AMPLMPModel(std::move(other)), model_(std::move(other.model_)) {
    other.model_= nullptr;
  }

  SCIPModel& operator=(SCIPModel& other) {
    if (this != &other)
    {
      AMPLMPModel::operator=(other);
      model_ = other.model_;
    }
    return *this;
  }

  SCIPModel& operator=(SCIPModel&& other) noexcept {
    if (this != &other) {
      AMPLMPModel::operator=(std::move(other));
      std::swap(model_, other.model_);
    }
    return *this;
  }

  using AMPLMPModel::getSolutionVector;

  const char* driver() { return "SCIP"; }

  Status::SolStatus getStatus() {
    SCIP_STATUS status = SCIPgetStatus(model_);
    switch (status) {
      case SCIP_STATUS_UNKNOWN:
        return Status::UNKNOWN;
      case SCIP_STATUS_USERINTERRUPT:
        return Status::INTERRUPTED;
      case SCIP_STATUS_NODELIMIT:
        return Status::LIMIT_NODE;
      case SCIP_STATUS_TOTALNODELIMIT:
        return Status::LIMIT_NODE;
      case SCIP_STATUS_STALLNODELIMIT:
        return Status::LIMIT_NODE;
      case SCIP_STATUS_TIMELIMIT:
        return Status::LIMIT_TIME;
      case SCIP_STATUS_MEMLIMIT:
        return Status::NOTMAPPED;
      case SCIP_STATUS_GAPLIMIT:
        return Status::NOTMAPPED;
      case SCIP_STATUS_SOLLIMIT:
        return Status::LIMIT_SOLUTION;
      case SCIP_STATUS_BESTSOLLIMIT:
        return Status::LIMIT_SOLUTION;
      case SCIP_STATUS_RESTARTLIMIT:
        return Status::LIMIT_ITERATION;
      case SCIP_STATUS_OPTIMAL:
        return Status::OPTIMAL;
      case SCIP_STATUS_INFEASIBLE:
        return Status::INFEASIBLE;
      case SCIP_STATUS_UNBOUNDED:
      case SCIP_STATUS_INFORUNBD:
        return Status::UNBOUNDED;
      case SCIP_STATUS_TERMINATE:
        return Status::INTERRUPTED;
      }
    return Status::NOTMAPPED;
  }

  int getNumVars() {
    return SCIPgetProbData(model_)->nvars;
  }
  int getNumCons() {
    return SCIPgetNConss(model_);
  };
  double getObj() {
    return SCIPgetPrimalbound(model_);
  }
  int getSolution(int first, int length, double* sol) {
    for (int i = first; i < first + length; i++)
      sol[i] = SCIPgetSolVal(model_, SCIPgetBestSol(model_), SCIPgetProbData(model_)->vars[i]);
    return 0;
  }

  void optimize();

  std::string error(int code);

  // **************** SCIP-specific ****************

  /** Get an integer parameter (using SCIP C library name) */
  int getIntParam(const char* name) {
    int value;
    if (SCIPparamGetType(SCIPgetParam(model_, name))==SCIP_PARAMTYPE_BOOL) {
      SCIP_Bool buffer;
      SCIPgetBoolParam(model_, name, &buffer);
      value = (int)buffer;
    }
    else
      SCIPgetIntParam(model_, name, &value);

    return value;
  }
  /** Get a double parameter (using SCIP C library name) */
  double getDoubleParam(const char* name) {
    double value;
    SCIPgetRealParam(model_, name, &value);

    return value;
  }
  /** Get a textual parameter (using SCIP C library name) */
  std::string getStrParam(const char* name) {
    std::string value;
    char* buffer;
    SCIPallocBufferArray(model_, &buffer, SCIP_MAXSTRLEN);
    if (SCIPparamGetType(SCIPgetParam(model_, name))==SCIP_PARAMTYPE_CHAR)
      SCIPgetCharParam(model_, name, buffer);
    else
      SCIPgetStringParam(model_, name, &buffer);
    value = buffer;
    SCIPfreeBufferArray(model_, &buffer);

    return value;
  }
  /** Set an integer parameter (using SCIP C library name) */
  void setParam(const char* name, int value) {
    if (SCIPparamGetType(SCIPgetParam(model_, name))==SCIP_PARAMTYPE_BOOL)
      SCIPsetBoolParam(model_, name, value);
    else
      SCIPsetIntParam(model_, name, value);
  }
  /** Set a double parameter (using SCIP C library name) */
  void setParam(const char* name, double value) {
    SCIPsetRealParam(model_, name, value);
  }
  /** Set a textual parameter (using SCIP C library name) */
  void setParam(const char* name, const char* value) {
    if (SCIPparamGetType(SCIPgetParam(model_, name))==SCIP_PARAMTYPE_CHAR)
      SCIPsetCharParam(model_, name, value[0]);
    else
      SCIPsetStringParam(model_, name, value);
  }

  /** Get the pointer to the native C SCIP_Model structure */
  SCIP* getSCIPmodel() {
    return model_;
  }

  ~SCIPModel();

  /**Set an integer parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    int value) {
    if (param == SolverParams::INT_LP_Algorithm)
      value = LPalgorithmMap[value];
    setParam(getSCIPParamAlias(param), value);
  }
  /**Set a double parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    double value) {    
    setParam(getSCIPParamAlias(param), value);
  }

  /**Get an integer parameter using ampls aliases*/
  int getAMPLIntParameter(SolverParams::SolverParameters params) {
    return getIntParam(getSCIPParamAlias(params));
  }
  /**Get a double parameter using ampls aliases*/
  double getAMPLDoubleParameter(SolverParams::SolverParameters params) {
    return getDoubleParam(getSCIPParamAlias(params));
  }


  /** Get an integer attribute using ampls aliases */
  int getAMPLIntAttribute(SolverAttributes::Attribs attrib) {
    throw ampls::AMPLSolverException("Not supported");
  }
  /** Get a double attribute using ampls aliases */
  double getAMPLDoubleAttribute(SolverAttributes::Attribs attrib) {
    double val;
    switch (attrib)
    {
    case SolverAttributes::DBL_RelMIPGap:
      double obj;
      val = SCIPgetDualbound(getSCIPmodel());
      obj = SCIPgetPrimalbound(getSCIPmodel());
      return impl::calculateRelMIPGAP(obj, val);
    case SolverAttributes::DBL_CurrentObjBound:
      val = SCIPgetDualbound(getSCIPmodel());
      break;
    default:
      throw ampls::AMPLSolverException("Not supported");
    }
    return val;
  }


  int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {
    char scipsense = 'G';//SCIPCallback::toSCIPSense(sense);

    SCIP_COL** scip_cols = NULL;
    SCIPallocBufferArray(getSCIPmodel(), &scip_cols, numnz);
    for (size_t i = 0; i < numnz; i++)
      scip_cols[i] = SCIPvarGetCol(SCIPgetProbData(getSCIPmodel())->vars[const_cast<int*>(vars)[i]]);

    SCIP_ROW* row;
    if (scipsense == 'E')
      SCIPcreateRowUnspec(getSCIPmodel(), &row, name, numnz, scip_cols, const_cast<double*>(coefficients), rhs, rhs, FALSE, FALSE, FALSE);
    else if (scipsense == 'G')
      SCIPcreateRowUnspec(getSCIPmodel(), &row, name, numnz, scip_cols, const_cast<double*>(coefficients), -1*getDoubleParam("numerics/infinity"), rhs, FALSE, FALSE, FALSE);
    else  
      SCIPcreateRowUnspec(getSCIPmodel(), &row, name, numnz, scip_cols, const_cast<double*>(coefficients), rhs, getDoubleParam("numerics/infinity"), FALSE, FALSE, FALSE);
    SCIPaddRow(getSCIPmodel(), row, TRUE, NULL);
    SCIPreleaseRow(getSCIPmodel(), &row);

    SCIPfreeBufferArray(getSCIPmodel(), &scip_cols);
    return getNumCons()-1;
  }

  int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {

    SCIP_VARTYPE vartype;

    switch (type)
    {
      case VarType::Continuous:
        vartype = SCIP_VARTYPE_CONTINUOUS;
        break;
      case VarType::Binary:
        vartype = SCIP_VARTYPE_BINARY;
        break;
      case VarType::Integer:
        vartype = SCIP_VARTYPE_INTEGER;
        break;
    }

    SCIP_VAR* var = NULL;

    SCIPcreateVarBasic(getSCIPmodel(), &var, name, lb, ub, objcoeff, vartype);
    int status = 0;
    if (numnz > 0) {
      for (size_t i = 0; i < numnz; i++) {
        SCIPaddVarToRow(getSCIPmodel(), SCIPconsGetRow(getSCIPmodel(), SCIPgetProbData(getSCIPmodel())->linconss[const_cast<int*>(cons)[i]]), var, const_cast<double*>(coefficients)[i]);
      }
    }

    SCIPaddVar(getSCIPmodel(), var);
    SCIPreleaseVar(getSCIPmodel(), &var);
    
    if (status != 0)
      throw ampls::AMPLSolverException::format("Could not add variable, code: %d.", status);;
    return getNumVars()-1;
  }

  std::vector<double> getConstraintsValueImpl(int offset, int length);
  std::vector<double> getVarsValueImpl(int offset, int length);


};
} // namespace
#endif // SCIP_INTERFACE_H_INCLUDE_
