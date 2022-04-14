#ifndef GUROBIDIRECT_INTERFACE_H_INCLUDE_
#define GUROBIDIRECT_INTERFACE_H_INCLUDE_

#ifdef _WIN32
  #define ENTRYPOINT __declspec(dllimport)
  #define API __declspec(dllexport)
#else
  #define ENTRYPOINT
#define API 
#endif

#include "ampls/ampls.h"
#include "gurobidirect_callback.h"
#include "gurobi_c.h"

#include <string>
#include <map>
#include <mutex>
#include <climits>  // for INT_MAX


namespace ampls
{
namespace grbdirect
{
  namespace impl
  {
    /// An AMPLS solver instance
    typedef struct AMPLS_MP_Solver_T {
      /// AMPLS internal info
      void* internal_info_;
      /// Extra info, managed by the specific solver
      void* solver_info_;
      /// User info, free to assign
      void* user_info_;
    } AMPLS_MP_Solver;

    /* Define a macro to do our error checking */
    #define AMPLSGRBERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing " #name":\n%s", error(status).c_str());

    extern "C" {
      // Imported from the GUROBI direct driver
      ENTRYPOINT GRBmodel* AMPLdirectloadmodel(int argc, char** argv, void** slvout);
      ENTRYPOINT void AMPLdirectwritesolution(void* slv);
      ENTRYPOINT void AMPLdirectclosesolver(void* slv);
      //ENTRYPOINT void freeASL(ASL** aslp);
     // ENTRYPOINT const char* AMPLGRBgetUinfo(ASL* asl);
    }
    // Forward declarations
    int callback_wrapper(GRBmodel* model, void* cbdata, int where, void* usrdata);
  }
}


class GurobiDirectModel;
class Callback;

/**
Encapsulates the main environment of the gurobi driver
*/
class GurobiDirectDrv : public impl::SolverDriver<GurobiDirectModel>  {


  void freeGurobiEnv();

  GurobiDirectModel loadModelImpl(char** args);
public:
  /**
  * Load a model from an NL file.
  * Mappings between solver row and column numbers and AMPL names are
  * available only if the row and col files have been generated as well,
  * by means of the ampl option `option auxfiles cr;` before writing the NL file.
  */
  GurobiDirectModel loadModel(const char* modelName);

  ~GurobiDirectDrv();
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
class GurobiDirectModel : public AMPLModel {
  friend GurobiDirectDrv;

  // Map for solver parameters
  std::map<int, const char*> parametersMap = {
     {SolverParams::INT_SolutionLimit , GRB_INT_PAR_SOLUTIONLIMIT},
     {SolverParams::DBL_MIPGap , GRB_DBL_PAR_MIPGAP},
     {SolverParams::DBL_TimeLimit , GRB_DBL_PAR_TIMELIMIT},
     {SolverParams::INT_LP_Algorithm, GRB_INT_PAR_METHOD}
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
     {SolverAttributes::DBL_RelMIPGap, GRB_DBL_ATTR_MIPGAP},
    {SolverAttributes::DBL_CurrentObjBound, GRB_DBL_ATTR_OBJBOUND}
  };
  const char* getGRBAttribAlias(SolverAttributes::Attribs attrib)
  {
    auto cpxParam = attribsMap.find(attrib);
    if (cpxParam != attribsMap.end())
      return cpxParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  mutable bool copied_;
  GRBmodel* GRBModel_;
  void* solver_;
  int lastErrorCode_;

  GurobiDirectModel() : AMPLModel(), copied_(false), GRBModel_(NULL),
    solver_(NULL), lastErrorCode_(0) {}

  // Interface implementation
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);
  void writeSolImpl(const char* solFileName);
public:
  void enableLazyConstraints()
  {
    setParam(GRB_INT_PAR_LAZYCONSTRAINTS, 1);
  }
  GurobiDirectModel(const GurobiDirectModel& other) :
    AMPLModel(other), copied_(false), GRBModel_(other.GRBModel_), 
    solver_(other.solver_), lastErrorCode_(other.lastErrorCode_)
  {
    other.copied_ = true;
  }
  using AMPLModel::getSolutionVector;

  const char* driver() { return "Gurobi"; }

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
  int getNumCons() {
    return getIntAttr(GRB_INT_ATTR_NUMCONSTRS);
  };
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
    int status = GRBgetintparam(GRBgetenv(GRBModel_), name, &v);
    AMPLSGRBERRORCHECK("GRBgetintparam")
    return v;
  }
  /** Get a double parameter (using gurobi C library name) */
  double getDoubleParam(const char* name) {
    double v;
    int status = GRBgetdblparam(GRBgetenv(GRBModel_), name, &v);
    AMPLSGRBERRORCHECK("GRBgetdblparam")
    return v;
  }
  /** Get a textual parameter (using gurobi C library name) */
  char* getStrParam(const char* name) {
    char* v = NULL;
    int status = GRBgetstrparam(GRBgetenv(GRBModel_), name, v);
    AMPLSGRBERRORCHECK("GRBgetstrparam")
    return v;
  }
  /** Set an integer parameter (using gurobi C library name) */
  void setParam(const char* name, int value) {
    int status = GRBsetintparam(GRBgetenv(GRBModel_), name, value);
    AMPLSGRBERRORCHECK("GRBsetintparam")
  }
  /** Set a double parameter (using gurobi C library name) */
  void setParam(const char* name, double value) {
    int status = GRBsetdblparam(GRBgetenv(GRBModel_), name, value);
    AMPLSGRBERRORCHECK("GRBsetdblparam")
  }
  /** Set a textual parameter (using gurobi C library name) */
  void setParam(const char* name, const char* value) {
    int status = GRBsetstrparam(GRBgetenv(GRBModel_), name, value);
    AMPLSGRBERRORCHECK("GRBsetdblparam")
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

  ~GurobiDirectModel();

  /**Set an integer parameter using ampls aliases*/
  void setAMPLsParameter(SolverParams::SolverParameters param,
    int value) {
    if (param == SolverParams::INT_LP_Algorithm)
      value = LPalgorithmMap[value];
    setParam(getGRBParamAlias(param), value);
  }
  /**Set a double parameter using ampls aliases*/
  void setAMPLsParameter(SolverParams::SolverParameters param,
    double value) {    
    setParam(getGRBParamAlias(param), value);
  }

  /**Get an integer parameter using ampls aliases*/
  int getAMPLsIntParameter(SolverParams::SolverParameters params) {
    return getIntParam(getGRBParamAlias(params));
  }
  /**Get a double parameter using ampls aliases*/
  double getAMPLsDoubleParameter(SolverParams::SolverParameters params) {
    return getDoubleParam(getGRBParamAlias(params));
  }

  /** Get an integer attribute using ampls aliases */
  int getAMPLsIntAttribute(SolverAttributes::Attribs attrib) {
    return getIntAttr(getGRBAttribAlias(attrib));
  }
  /** Get a double attribute using ampls aliases */
  double getAMPLsDoubleAttribute(SolverAttributes::Attribs attrib) {
    switch (attrib)
    {
    case SolverAttributes::DBL_RelMIPGap:
      return impl::calculateRelMIPGAP(getDoubleAttr(GRB_DBL_ATTR_OBJVAL),
        getDoubleAttr(GRB_DBL_ATTR_OBJBOUND));
    default:
      return getDoubleParam(getGRBAttribAlias(attrib));
    }
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
    getDoubleAttrArray(GRB_DBL_ATTR_RC, min, n, values.data());
    return values;
  }

  int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {
    char grbsense = GurobiDirectCallback::toGRBSense(sense);
    int status = GRBaddconstr(getGRBmodel(), numnz, const_cast<int*>(vars),
      const_cast<double*>(coefficients), grbsense, rhs, name);
    status=  GRBupdatemodel(getGRBmodel());
    return getNumCons()-1;
  }

  int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {
    char t;
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
  }

  std::vector<double>  getConstraintsValueImpl(int offset, int length);
  std::vector<double> getVarsValueImpl(int offset, int length);


};
} // namespace
#endif // GUROBI_INTERFACE_H_INCLUDE_
