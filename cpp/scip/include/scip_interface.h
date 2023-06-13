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
      ENTRYPOINT ampls::impl::mp::AMPLS_MP_Solver* AMPLSOpen_scip(int, char**);
    }
    // Forward declarations
    void cut_callback_wrapper(void* osisolver, void* osicuts, void* appdata, int level, int pass);
    void callback_wrapper(SCIP* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
      int nchar, char** cvec);
  }
}


class SCIPModel;
class Callback;

/**
Encapsulates the main environment of the cbc driver
*/
class SCIPDrv : public impl::SolverDriver<SCIPModel>  {


  void freeSCIPEnv();

  SCIPModel loadModelImpl(char** args, const char** options);
public:
  /**
  * Load a model from an NL file.
  * Mappings between solver row and column numbers and AMPL names are
  * available only if the row and col files have been generated as well,
  * by means of the ampl option `option auxfiles cr;` before writing the NL file.
  */
  SCIPModel loadModel(const char* modelName);

  ~SCIPDrv();
};

/**
Encapsulates all the instance level information for a SCIP model,
namely the SCIP object and the relative ASL.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the SCIP object and the ASL structures.
Note that if we don't want to use the writesol function, we don't really
need the ASL reference after creating the SCIP model object, so
we could use directly the C pointer to SCIP.
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
  const char* getGRBParamAlias(SolverParams::SolverParameters params)
  {
    auto cpxParam = parametersMap.find(params);
    if (cpxParam != parametersMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  // Map for solver attributes
  std::map<int, const char*> attribsMap = {
     {SolverAttributes::DBL_RelMIPGap, "GRB_DBL_ATTR_MIPGAP"},
    {SolverAttributes::DBL_CurrentObjBound, "GRB_DBL_ATTR_OBJBOUND"}
  };
  const char* getGRBAttribAlias(SolverAttributes::Attribs attrib)
  {
    auto cpxParam = attribsMap.find(attrib);
    if (cpxParam != attribsMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  SCIP* model_;

  SCIPModel() : AMPLMPModel(), model_(NULL) {}
  SCIPModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile, 
    const char** options) : AMPLMPModel(s, nlfile, options) {
    model_ = (SCIP*)impl::scip::AMPLSGetModel_scip(s);
    
  }
  // Interface implementation
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);
  void writeSolImpl(const char* solFileName);
public:
  using Driver = ampls::SCIPDrv;

  void enableLazyConstraints()  {
    //Cbc_setParameter(model_, "preprocess", "off");
  }

  SCIPModel(const SCIPModel& other) :
    AMPLMPModel(other), model_(other.model_)
  { }
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
      model_ = std::move(other.model_);
    }
    return *this;
  }

  using AMPLMPModel::getSolutionVector;

  const char* driver() { return "SCIP"; }

  Status::SolStatus getStatus() {/*
  
    if (Cbc_isProvenOptimal(model_))
      return Status::OPTIMAL;
    if (Cbc_isProvenInfeasible(model_))
      return Status::INFEASIBLE;
    if (Cbc_isContinuousUnbounded(model_))
      return  Status::UNBOUNDED;
    switch (Cbc_status(model_)) {
    case -1:
      return Status::UNKNOWN;
    case 1:
      return Status::LIMIT_TIME;
    case 2:
      return  Status::UNKNOWN;
    case 5:
      return Status::INTERRUPTED;
    default:
      return Status::UNKNOWN;
    }*/
    return Status::UNKNOWN;
  }

  int getNumVars() {
    return SCIPgetNVars(model_);
  }
  int getNumCons() {
    return SCIPgetNConss(model_);
  };
  double getObj() {
    double obj;
    obj = SCIPgetSolOrigObj(model_, SCIPgetBestSol(model_));
    return obj;
  }
  int getSolution(int first, int length, double* sol) {
    //auto variables = Cbc_getColSolution(model_);
    //for (int i = first; i < first + length; i++)
    //  sol[i] = variables[i + first];
    return 0;
  }

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
    setParam(getGRBParamAlias(param), value);
  }
  /**Set a double parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    double value) {    
    setParam(getGRBParamAlias(param), value);
  }

  /**Get an integer parameter using ampls aliases*/
  int getAMPLIntParameter(SolverParams::SolverParameters params) {
    return getIntParam(getGRBParamAlias(params));
  }
  /**Get a double parameter using ampls aliases*/
  double getAMPLDoubleParameter(SolverParams::SolverParameters params) {
    return getDoubleParam(getGRBParamAlias(params));
  }

  /** Get an integer attribute using ampls aliases */
  //int getAMPLIntAttribute(SolverAttributes::Attribs attrib) {
  //  return getIntAttr(getGRBAttribAlias(attrib));
  //}
  /** Get a double attribute using ampls aliases */
  //double getAMPLDoubleAttribute(SolverAttributes::Attribs attrib) {
  //    return getDoubleParam(getGRBAttribAlias(attrib));
  //}

  std::vector<double> getConstraintsValueImpl(const std::vector<int>& indices) {
    int min = INT_MAX, max = INT_MIN;
    for (int i : indices)
    {
      if (i < min) min = i;
      if (i > max) max = i;
    }
    int n = max - min;
    std::vector<double> values;
    values.resize(n);
   // getDoubleAttrArray(GRB_DBL_ATTR_RC, min, n, values.data());
    //TODO
    return values;
  }

  int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {
    char grbsense = SCIPCallback::toSCIPSense(sense);
    //TODO
    //int status = GRBaddconstr(getGRBmodel(), numnz, const_cast<int*>(vars),
    //  const_cast<double*>(coefficients), grbsense, rhs, name);
    //status=  GRBupdatemodel(getGRBmodel());
    return getNumCons()-1;
  }

  int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {
    char t;
    ///TODO
    /*
    switch (type)
    {
      case VarType::Continuous:
        t = GRB_CONTINUOUS;
        break;
      case VarType::Binary:
        t = GRB_BINARY;
        break;
      case VarType::Integer:
        t = GRB_INTEGER;
        break;
    }
    int status = 0;
    if (numnz == 0)
    {
       status = GRBaddvar(getGRBmodel(), 0, NULL, NULL, objcoeff, lb, ub, t, name);
    }
    else {
      status = GRBaddvar(getGRBmodel(), numnz, const_cast<int*>(cons),
        const_cast<double*>(coefficients), objcoeff, lb, ub, t, name);
    }
    if (status != 0)
      throw ampls::AMPLSolverException::format("Could not add variable, code: %d.", status);;
    GRBupdatemodel(getGRBmodel());
    return getNumVars()-1;
    */

    return 0;

  }

  std::vector<double>  getConstraintsValueImpl(int offset, int length);
  std::vector<double> getVarsValueImpl(int offset, int length);


};
} // namespace
#endif // CBC_INTERFACE_H_INCLUDE_
