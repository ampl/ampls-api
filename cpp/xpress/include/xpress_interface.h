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
      ENTRYPOINT XPRSprob AMPLSGetModel_xpress(void* slv);
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
      // Declares an optimal node callback function, called after an optimal solution for the current 
      // node has been found during the Branch and Bound search.
      // feas: The feasibility status.If set to a nonzero value by the user, the current node will be declared infeasible.
      static void XPRS_CC optnode_callback_wrapper(XPRSprob my_prob, void* my_object, int* feas);
      // Declares a node selection callback function. 
      // This is called every time the code backtracks to select a new node during the MIP search.
      // nodnum 
      // node: a pointer to the number of the node, nodnum, selected by the Optimizer.
      // By changing the value pointed to by this argument, the selected node may be changed with 
      // this function.
      // static void XPRS_CC chgnode_callback_wrapper(XPRSprob prob, void* object,
      //   int* node);

      // Declares a user infeasible node callback function, 
      // called after the current node has been found to be infeasible during the Branch and Bound search.
      // static void XPRS_CC infnode_callback_wrapper(XPRSprob prob, void* object);

      // Declares a user node cutoff callback function, 
      // called every time a node is cut off as a result of an improved integer solution being found 
      // during the Branch and Bound search.
      // node The number of the node that is cut off.
      // static void XPRS_CC nodecutoff_callback_wrapper(XPRSprob prob, void* object, int node);

      // Declares a branching variable callback function, called every time a new branching variable 
      // is set or selected during the MIP search.
      // entity: A pointer to the variable or set on which to branch.Ordinary global variables are identified by their column index, i.e. 0, 1, ...(COLS - 1) and by their set index, i.e. 0, 1, ..., (SETS - 1).
      // up: If entity is a variable, this is 1 if the upward branch is to be made first, or 0 otherwise.If entity is a set, this is 3 if the upward branch is to be made first, or 2 otherwise.
      // estdeg: The estimated degradation at the node.
      // static void XPRS_CC chgbranch_callback_wrapper(XPRSprob prob, void* vdata,
        // int* entity, int* up, double* estdeg);

      // Declares a preprocess node callback function, called before the node has been optimized,
      // so the solution at the node will not be available.
      // feas: the feasibility status, if set to a nonzero value by the user, the current node 
      // will be declared infeasible by the optimizer.
      // static void XPRS_CC prenode_callback_wrapper(XPRSprob prob, void* data, int* feas);


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
  XPRESSModel loadModelImpl(char** args);
public:
  /**
  * Load a model from an NL file.
  * Mappings between solver row and column numbers and AMPL names are
  * available only if the row and col files have been generated as well,
  * by means of the ampl option `option auxfiles cr;` before writing the NL file.
  */
  XPRESSModel loadModel(const char* modelName);
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
  clock_t tStart_;
  XPRESSDrv  driver_;
  

  XPRESSModel() : AMPLMPModel(),
    prob_(NULL), tStart_(0) {}
  XPRESSModel(impl::mp::AMPLS_MP_Solver* s, const char* nlfile) : AMPLMPModel(s,nlfile) {
    prob_ = impl::xpress::AMPLSGetModel_xpress(s);
  }

  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);

  // Parameters map
  std::map<int, int> parametersMap = {
     {SolverParams::INT_SolutionLimit , XPRS_MAXMIPSOL},
     {SolverParams::DBL_MIPGap , XPRS_MIPRELSTOP},
     {SolverParams::DBL_TimeLimit , XPRS_MAXTIME},
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
  std::map<int, int> attribsMap = { };
  int getXPRESSParamAlias(SolverAttributes::Attribs attrib) const
  {
    auto xpressParam = parametersMap.find(attrib);
    if (xpressParam != parametersMap.end())
      return xpressParam->second;
    throw AMPLSolverException("Not implemented!");
  }
 
public:
  using Driver = ampls::XPRESSDrv;

  XPRESSModel(const XPRESSModel &other) : AMPLMPModel(other), 
    prob_(other.prob_), tStart_(other.tStart_) {
    driver_ = other.driver_;
  }
  XPRESSModel(XPRESSModel&& other) noexcept :
    AMPLMPModel(std::move(other)), prob_(std::move(other.prob_)),
    tStart_(std::move(other.tStart_)), driver_(other.driver_) { 
    other.prob_ = nullptr;
  }
  
  XPRESSModel& operator=(XPRESSModel &other) {
    if (this != &other)
    {
      AMPLMPModel::operator=(other);
      prob_ = other.prob_;
      tStart_ = other.tStart_;
      driver_ = other.driver_;
    }
    return *this;
  }

  XPRESSModel& operator=(XPRESSModel&& other) noexcept {
    if (this != &other) {
      AMPLMPModel::operator=(std::move(other));
      prob_ = std::move(other.prob_);
      other.prob_ = nullptr;
      tStart_ = other.tStart_;
      driver_ = std::move(other.driver_);
    }
    return *this;
  }


  const char* driver() { return "Xpress"; }

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

  int optimize();

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
    setParam(XPRS_PRESOLVE,0);
    setParam(XPRS_MIPPRESOLVE, 0);
    setParam(XPRS_SYMMETRY, 0); 
  };

  /**Set an integer parameter using ampls aliases*/
  void setAMPLSParameter(SolverParams::SolverParameters param,
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
  void setAMPLSParameter(SolverParams::SolverParameters param,
    double value) {
    setParam(getXPRESSParamAlias(param), value);
  }

  /**Get an integer parameter using ampls aliases*/
  int getAMPLSIntParameter(SolverParams::SolverParameters params) {
    return getIntParam(getXPRESSParamAlias(params));
  }
  /**Get a double parameter using ampls aliases*/
  double getAMPLSDoubleParameter(SolverParams::SolverParameters params) {
    return getDoubleParam(getXPRESSParamAlias(params));
  }


  /** Get an integer attribute using ampls aliases */
  int getAMPLSIntAttribute(SolverAttributes::Attribs attrib) {
    return getIntAttr(getXPRESSParamAlias(attrib));
  }
  /** Get a double attribute using ampls aliases */
  double getAMPLSDoubleAttribute(SolverAttributes::Attribs attrib) {
    switch (attrib)
    {
    case SolverAttributes::DBL_RelMIPGap:
      return impl::calculateRelMIPGAP(getDoubleAttr(XPRS_MIPOBJVAL),
        getDoubleAttr(XPRS_BESTBOUND));
    case SolverAttributes::DBL_CurrentObjBound:
      return getDoubleAttr(XPRS_BESTBOUND);
    default:
      return getDoubleAttr(getXPRESSParamAlias(attrib));
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
    double objd[] = { objcoeff };
    double ubd[] = { ub };
    double lbd[] = { lb };
    char* named[] = { const_cast<char*>(name) };
    int status = XPRSaddcols(prob_, 1, numnz, objd, 0, cons, coefficients, lbd, ubd);
    AMPLSXPRSERRORCHECK("XPRSaddcols")
    char varType[] = { toXPRESSType[(int)type] };
    int indices[] = { getNumVars() - 1 };
    XPRSchgcoltype(prob_, 1, indices, varType);
    return indices[0];
  }
  std::vector<double> getConstraintsValueImpl(int offset, int length);
  std::vector<double> getVarsValueImpl(int offset, int length);

};

} // namespace
#endif // XPRESS_INTERFACE_H_INCLUDE_