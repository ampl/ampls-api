#ifndef XPRESS_INTERFACE_H_INCLUDE_
#define XPRESS_INTERFACE_H_INCLUDE_

#include <string>
#include <map>
#include <mutex>

#include "xprs.h"

#include "ampls/ampls.h"
#include "xpress_callback.h"

#include "time.h"

struct ASL;


namespace ampls
{


  namespace impl
  {
    namespace xpress
    {

    /* Define a macro to do our error checking */
    #define AMPLSXPRSERRORCHECK(name)  \
    if (status)  \
      throw ampls::AMPLSolverException::format("Error executing " #name":\n%s", error(status).c_str());


    extern "C" {
      // Imported from the xpress driver
      ENTRYPOINT void AMPLSClose_xpress(void* slv);
      ENTRYPOINT void* AMPLSGetModel_xpress(void* slv);
      ENTRYPOINT void* AMPLSOpen_xpress(int, char**);
    }
    
  
    class XPRSCBWrap
    {
    public:
      static XPRESSCallback* setDefaultCB(XPRSprob prob, void* object, XPRESSWhere whereFrom, int capabilities=0);

      // Declares an output callback function, called every time a text line is output by the Optimizer.
      // msg: A null terminated character array(string) containing the message, which may simply be a new line.
      // len: The length of the message string, excluding the null terminator.
      // msgtype: Indicates the type of output message:
      //     1 information messages;
      //     2 (not used);
      //     3 warning messages;
      //     4 error messages
      // A negative value indicates that the Optimizer is about to finish and the buffers should be flushed at this time if the output is being redirected to a file.
      static void XPRS_CC message_callback_wrapper(XPRSprob prob, void* object, const char* msg, int len, int msgtype);
      // Declares a user integer solution callback function, 
      // called every time an integer solution is found by heuristics or during the Branch and Bound 
      static void XPRS_CC  intsol_callback_wrapper(XPRSprob prob, void* object);
      // Called every new node
      static void XPRS_CC  newnode_callback_wrapper(XPRSprob cbprob, void* cbdata, int parentnode, int node, int branch);
      // Declares an optimal node callback function, called after an optimal solution for the current 
      // node has been found during the Branch and Bound search.
      // feas: The feasibility status.If set to a nonzero value by the user, the current node will be declared infeasible.
      static void XPRS_CC optnode_callback_wrapper(XPRSprob my_prob, void* my_object, int* feas);

      static void XPRS_CC preintsol_callback_wrapper(XPRSprob prob, void* object, int soltype, int* p_reject, double* p_cutoff);
    
    };
  }
}

class XPRESSCallback;
class XPRESSModel;

/**
Encapsulates the main environment of the xpress driver; the environment is then
associated with the model being instantiated and deleted with it
*/
class XPRESSDrv : public impl::SolverDriver<XPRESSModel> {
  mutable bool owning_;
  void freeXPRESSEnv();
  XPRESSModel loadModelImpl(char** args, const char** options);
public:
  ~XPRESSDrv();

  XPRESSDrv() : owning_(false) {}

  XPRESSDrv(XPRESSDrv& other) : owning_(other.owning_){
    other.owning_ = false;
  }
  XPRESSDrv& operator=(const XPRESSDrv& other) {
    owning_ = other.owning_;
    other.owning_ = false;
    return *this;
  }
};

/**
Encapsulates all the instance level information for a CPLEX model,
namely the CPLEX object, the relative ASL and all the locals of the
driver up to the moment in which optimize would be called.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the relative structures.
*/
class XPRESSModel : public AMPLMPModel {
  friend XPRESSDrv;
  friend XPRESSCallback;


  XPRSprob prob_;
  XPRESSDrv  driver_;
  

  XPRESSModel() : AMPLMPModel(),
    prob_(NULL) {}
  XPRESSModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile, const char** options) : AMPLMPModel(s,nlfile, options) {
    prob_ = static_cast<XPRSprob>(impl::xpress::AMPLSGetModel_xpress(s));
  }

  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);

  // Parameters map
  std::map<int, int> parametersMap = {
     {SolverParams::INT_SolutionLimit , XPRS_MAXMIPSOL},
     {SolverParams::DBL_MIPGap , XPRS_MIPRELSTOP},
     {SolverParams::DBL_TimeLimit , XPRS_TIMELIMIT},
     {SolverParams::INT_LP_Algorithm , XPRS_ALGORITHM}
  };
  const int LPalgorithmMap[4] = { -1, 3, 2, 4 };
  int getXPRESSParamAlias(SolverParams::SolverParameters params) const
  {
    auto xpressParam = parametersMap.find(params);
    if (xpressParam != parametersMap.end())
      return xpressParam->second;
    throw AMPLSolverException("Not implemented!");
  }

  // Attributes map
  std::map<int, int> attribsMap = {
    {SolverAttributes::INT_NumIntegerVars, XPRS_MIPENTS}
  };
  int getXPRESSAttribAlias(SolverAttributes::Attribs attrib) const
  {
    auto xpressAttrib = attribsMap.find(attrib);
    if (xpressAttrib != attribsMap.end())
      return xpressAttrib->second;
    throw AMPLSolverException("Not implemented!");
  }
 
public:
  using Driver = ampls::XPRESSDrv;

  XPRESSModel(const XPRESSModel &other) : AMPLMPModel(other), 
    prob_(other.prob_) {
    driver_ = other.driver_;
  }
  XPRESSModel(XPRESSModel&& other) noexcept :
    AMPLMPModel(std::move(other)), prob_(std::move(other.prob_)),
    driver_(other.driver_) { 
    other.prob_ = nullptr;
  }
  
  XPRESSModel& operator=(XPRESSModel &other) {
    if (this != &other)
    {
      AMPLMPModel::operator=(other);
      prob_ = other.prob_;
      driver_ = other.driver_;
    }
    return *this;
  }

  XPRESSModel& operator=(XPRESSModel&& other) noexcept {
    if (this != &other) {
      AMPLMPModel::operator=(std::move(other));
      prob_ = std::move(other.prob_);
      other.prob_ = nullptr;
      driver_ = std::move(other.driver_);
    }
    return *this;
  }


  const char* driver() { return "xpress"; }

  double infinity() override { return XPRS_PLUSINFINITY; }
  double negInfinity() override { return XPRS_MINUSINFINITY; }

  Status::SolStatus getStatus() {
    if (!isMIP())
    {
      int stat = getIntAttr(XPRS_LPSTATUS);
      switch (stat)
      {
      case XPRS_LP_UNSTARTED:
        return Status::UNKNOWN;
      case XPRS_LP_OPTIMAL:
        return Status::OPTIMAL;
      case XPRS_LP_INFEAS:
        return Status::INFEASIBLE;
      case XPRS_LP_UNBOUNDED:
        return Status::UNBOUNDED;
      case XPRS_LP_UNFINISHED:
        return Status::UNKNOWN;
      default:
        return Status::NOTMAPPED;
        // XPRS_LP_CUTOFF
        // XPRS_LP_CUTOFF_IN_DUAL
        // XPRS_LP_UNSOLVED
        // XPRS_LP_NONCONVEX
      }
    }
    else
    {
      int stat = getIntAttr(XPRS_MIPSTATUS);
      switch (stat)
      {
      case XPRS_MIP_NOT_LOADED:
        return Status::UNKNOWN;
      case XPRS_MIP_OPTIMAL:
        return Status::OPTIMAL;
      case XPRS_MIP_INFEAS:
        return Status::INFEASIBLE;
      case XPRS_MIP_UNBOUNDED:
        return Status::UNBOUNDED;
      case XPRS_MIP_SOLUTION:
      case XPRS_MIP_LP_OPTIMAL:
        return Status::INTERRUPTED;
      default:
        return Status::NOTMAPPED;
        // XPRS_MIP_NO_SOL_FOUND
        // XPRS_MIP_LP_NOT_OPTIMAL:
      }
    }
  }
  int getNumVars() {
    return getIntAttr(XPRS_ORIGINALCOLS);
  }

  int getNumCons() {
    return getIntAttr(XPRS_ORIGINALROWS);
  }
  double getObj() {
    if(isMIP())
      return getDoubleAttr(XPRS_MIPOBJVAL);
    else
      return getDoubleAttr(XPRS_LPOBJVAL);
  }

  int getSolution(int first, int length, double* sol) {
    int nvars = getNumVars();
    if (length < nvars)
      throw AMPLSolverException::format("Must allocate an array of at least %d elements.", nvars);
    return XPRSgetsol(prob_, sol, NULL, NULL, NULL);
  }
  std::string error(int code)
  {
    char errmsg[512];
    XPRSgetlasterror(prob_, errmsg);
    return errmsg;
  }


  // ******************* XPRESS specific *******************
  /** Get a pointer to the underlying XPRESSprob object */
  XPRSprob getXPRSprob() {
    return prob_;
  }
  /** Solve the problem */
  void optimize() {
    if (getIntAttr(XPRS_ORIGINALMIPENTS) > 0)
      XPRSmipoptimize(prob_, NULL);
    else
      XPRSlpoptimize(prob_, NULL);
  }
  /** Get an integer attribute identified by XPRESS native enum */
  int getIntAttr(int what) {
    int ret;
    int status = XPRSgetintattrib(prob_, what, &ret);
    AMPLSXPRSERRORCHECK("XPRSgetintattrib")
    return ret;
  }
  /** Get a double attribute  identified by XPRESS native enum*/
  double getDoubleAttr(int what) {
    double ret;
    int status = XPRSgetdblattrib(prob_, what, &ret);
    AMPLSXPRSERRORCHECK("XPRSgetdblattrib")
    return ret;
  }
  /** Return true if the problem is MIP*/
  bool isMIP() {
    return getIntAttr(XPRS_ORIGINALMIPENTS) > 0;
  }

  ~XPRESSModel() {
    if (copied_)
      return;
  }

  /** Set an integer XPRESS control parameter */
  void setParam(int XPRSParam, int value) {
    int status = XPRSsetintcontrol(prob_, XPRSParam, value);
    AMPLSXPRSERRORCHECK("XPRSsetintcontrol")
  }
  /** Set a double XPRESS control parameter */
  void setParam(int XPRSParam, double value) {
    int status = XPRSsetdblcontrol(prob_, XPRSParam, value);
    AMPLSXPRSERRORCHECK("XPRSsetdblcontrol")
  }

  /** Get an integer XPRESS control parameter */
  int getIntParam(int XPRSParam) {
    int value;
    int status= XPRSgetintcontrol(prob_, XPRSParam, &value);
    AMPLSXPRSERRORCHECK("XPRSgetintcontrol")
    return value;
  }

  /** Get a double XPRESS control parameter */
  double getDoubleParam(int XPRSParam) {
    double value;
    int status = XPRSgetdblcontrol(prob_, XPRSParam, &value);
    AMPLSXPRSERRORCHECK("XPRSgetdblcontrol")
    return value;
  }

  void enableLazyConstraints() {
    setParam(XPRS_PRESOLVE, 0);
    setParam(XPRS_MIPDUALREDUCTIONS, 0);
  };

  /**Set an integer parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    int value) {
    if (param == SolverParams::INT_LP_Algorithm)
    {
      if (value == LPAlgorithms::Auto)
        return; // No "automatic" in xpress
      value = LPalgorithmMap[value];
    }
    setParam(getXPRESSParamAlias(param), value);
  }
  /**Set a double parameter using ampls aliases*/
  void setAMPLParameter(SolverParams::SolverParameters param,
    double value) {
    setParam(getXPRESSParamAlias(param), value);
  }

  /**Get an integer parameter using ampls aliases*/
  int getAMPLIntParameter(SolverParams::SolverParameters params) {
    return getIntParam(getXPRESSParamAlias(params));
  }
  /**Get a double parameter using ampls aliases*/
  double getAMPLDoubleParameter(SolverParams::SolverParameters params) {
    return getDoubleParam(getXPRESSParamAlias(params));
  }


  /** Get an integer attribute using ampls aliases */
  int getAMPLIntAttribute(SolverAttributes::Attribs attrib) {
    return getIntAttr(getXPRESSAttribAlias(attrib));
  }
  /** Get a double attribute using ampls aliases */
  double getAMPLDoubleAttribute(SolverAttributes::Attribs attrib) {
    switch (attrib)
    {
    case SolverAttributes::DBL_RelMIPGap:
      return impl::calculateRelMIPGAP(getDoubleAttr(XPRS_MIPOBJVAL),
        getDoubleAttr(XPRS_BESTBOUND));
    case SolverAttributes::DBL_CurrentObjBound:
      return getDoubleAttr(XPRS_BESTBOUND);
    default:
      return getDoubleAttr(getXPRESSAttribAlias(attrib));
    }
    
  }



  int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {
    double rhsd[] = { rhs };
    char sensed[] = { XPRESSCallback::toXPRESSRowType[(int)sense] };
    int rowbegin[] = { 0 };
    char* named[] = { const_cast<char*>(name) };
    // TODO Handle Infinity
    int status =  XPRSaddrows(prob_,1, numnz, sensed, 
      rhsd, NULL, rowbegin, vars, coefficients);
    AMPLSXPRSERRORCHECK("XPRSaddrows")
    return getNumCons() - 1;
  }
  const char toXPRESSType[3] = { 'C', 'B', 'I'};
  int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {
    
    int start[] = { 0 };
    int status = XPRSaddcols(prob_, 1, numnz, &objcoeff, 
      start, cons, coefficients, &lb, &ub);
    AMPLSXPRSERRORCHECK("XPRSaddcols")
    char varType[] = { toXPRESSType[(int)type] };
    int indices[] = { getNumVars() - 1 };
    XPRSchgcoltype(prob_, 1, indices, varType);
    if (name != nullptr) 
      XPRSaddnames(prob_, XPRS_NAMES_COLUMN, name, indices[0], indices[0]+1);
    
    return indices[0];
  }
  std::vector<double> getConstraintsValueImpl(int offset, int length);
  std::vector<double> getVarsValueImpl(int offset, int length);

};

} // namespace
#endif // XPRESS_INTERFACE_H_INCLUDE_