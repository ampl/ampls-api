#ifndef CBC_INTERFACE_H_INCLUDE_
#define CBC_INTERFACE_H_INCLUDE_

#ifdef _WIN32
  #define ENTRYPOINT __declspec(dllimport)
  #define API __declspec(dllexport)
#else
  #define ENTRYPOINT
#define API 
#endif

#include "ampls/ampls.h"
#include "cbcmp_callback.h"

#include <string>
#include <map>
#include <mutex>
#include <climits>  // for INT_MAX


namespace ampls
{

  namespace impl
  {
    namespace cbcmp
    {
    /* Define a macro to do our error checking */
    #define AMPLSCBCERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing " #name":\n%s", error(status).c_str());

    extern "C" {
      ENTRYPOINT void AMPLSClose_cbcmp(void* slv);
      ENTRYPOINT void* AMPLSGetModel_cbcmp(void* slv);
      ENTRYPOINT void* AMPLSOpen_cbcmp(int, char**);
    }
    // Forward declarations
    void cut_callback_wrapper(void* osisolver, void* osicuts, void* appdata, int level, int pass);
    void callback_wrapper(Cbc_Model* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
      int nchar, char** cvec);
  }
}


class CbcModel;
class Callback;

/**
Encapsulates the main environment of the cbc driver
*/
class CbcDrv : public impl::SolverDriver<CbcModel>  {


  void freeCbcEnv();

  CbcModel loadModelImpl(char** args, const char** options);
public:
  ~CbcDrv();
};

/**
Encapsulates all the instance level information for a cbc model,
namely the GRBmodel object and the relative ASL.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the GRBmodel and the ASL structures.
Note that if we don't want to use the writesol function, we don't really
need the ASL reference after creating the Cbc model object, so
we could use directly the C pointer to GRBmodel.
*/
class CbcModel : public AMPLMPModel {
  friend CbcDrv;

  // Map for solver parameters
  std::map<int, const char*> parametersMap = {
     {SolverParams::INT_SolutionLimit , "0"},
     {SolverParams::DBL_MIPGap , "1"},
     {SolverParams::DBL_TimeLimit , "2"},
     {SolverParams::INT_LP_Algorithm, "3"}
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

  Cbc_Model* model_;

  CbcModel() : AMPLMPModel(), model_(NULL) {}
  CbcModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile,
    const char** options) : AMPLMPModel(s, nlfile, options) {
    model_ = impl::cbcmp::AMPLSGetModel_cbcmp(s);
    
  }
  // Interface implementation
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);
public:
  using Driver = ampls::CbcDrv;

  void enableLazyConstraints()  {
    Cbc_setParameter(model_, "preprocess", "off");
  }

  CbcModel(const CbcModel& other) :
    AMPLMPModel(other), model_(other.model_)
  { }
  CbcModel(CbcModel&& other) noexcept :
    AMPLMPModel(std::move(other)), model_(std::move(other.model_)) {
    other.model_= nullptr;
  }

  CbcModel& operator=(CbcModel& other) {
    if (this != &other)
    {
      AMPLMPModel::operator=(other);
      model_ = other.model_;
    }
    return *this;
  }

  CbcModel& operator=(CbcModel&& other) noexcept {
    if (this != &other) {
      AMPLMPModel::operator=(std::move(other));
      model_ = std::move(other.model_);
    }
    return *this;
  }

  using AMPLMPModel::getSolutionVector;

  const char* driver() { return "Cbc"; }

  void optimize();

  Status::SolStatus getStatus() {
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
    }
  }

  int getNumVars() {
    return Cbc_getNumCols(model_);
  }
  int getNumCons() {
    return Cbc_getNumRows(model_);
  };
  double getObj() {
    return Cbc_getObjValue(model_);
  }
  int getSolution(int first, int length, double* sol) {
    auto variables = Cbc_getColSolution(model_);
    for (int i = first; i < first + length; i++)
      sol[i] = variables[i + first];
    return 0;
  }

  std::string error(int code);

  // **************** Cbc-specific ****************

  /** Get an integer model attribute (using cbc C library name) */
  int getIntAttr(const char* name);
  /** Get a double model attribute (using cbc C library name) */
  double getDoubleAttr(const char* name);
  /** Get an integer array model attribute (using cbc C library name) */
  int getIntAttrArray(const char* name, int first, int length, int* arr);
  /** Get a double array model attribute (using cbc C library name) */
  int getDoubleAttrArray(const char* name, int first, int length, double* arr);

  /** Get an integer parameter (using cbc C library name) */
  int getIntParam(const char* name) {
    /*std::string argname = std::string("-") + name;
    for (int i = 0; (i < ((int)model_->cmdargs_.size()) - 1); ++i)
      if (argname == model_->cmdargs_[i])
      {
        return atoi(model_->cmdargs_[i + 1].c_str());
      }*/
    return -1;
  }
  /** Get a double parameter (using cbc C library name) */
  double getDoubleParam(const char* name) {
    /*
    std::string argname = std::string("-") + name;
    for (int i = 0; (i < ((int)model_->cmdargs_.size()) - 1); ++i)
      if (argname == model_->cmdargs_[i])
      {
        return atof(model_->cmdargs_[i + 1].c_str());
      }*/
    return -1;
  }
  /** Get a textual parameter (using cbc C library name) */
  std::string getStrParam(const char* name) {
    /*
    std::string argname = std::string("-") + name;
    for (int i = 0; (i < ((int)model_->cmdargs_.size()) - 1); ++i)
      if (argname == model_->cmdargs_[i])
      {
        return model_->cmdargs_[i + 1];
      }*/
    return "";
  }
  /** Set an integer parameter (using cbc C library name) */
  void setParam(const char* name, int value) {
    std::string s = std::to_string(value);
    Cbc_setParameter(model_, name, s.data());
  }
  /** Set a double parameter (using cbc C library name) */
  void setParam(const char* name, double value) {
    std::string s = std::to_string(value);
    Cbc_setParameter(model_, name, s.data());
  }
  /** Set a textual parameter (using cbc C library name) */
  void setParam(const char* name, const char* value) {
    Cbc_setParameter(model_, name, value);
  }

  /** Get the pointer to the native C Cbc_Model structure */
  Cbc_Model* getCBCmodel() {
    return model_;
  }
  ~CbcModel();

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
  int getAMPLIntAttribute(SolverAttributes::Attribs attrib) {
    return getIntAttr(getGRBAttribAlias(attrib));
  }
  /** Get a double attribute using ampls aliases */
  double getAMPLDoubleAttribute(SolverAttributes::Attribs attrib) {
      return getDoubleParam(getGRBAttribAlias(attrib));
  }

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
    char grbsense = CbcCallback::toCBCSense(sense);
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
