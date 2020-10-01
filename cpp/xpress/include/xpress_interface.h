#ifndef CPLEX_INTERFACE_H_INCLUDE_
#define CPLEX_INTERFACE_H_INCLUDE_

#ifdef _WIN32
#define ENTRYPOINT __declspec(dllimport)
#else
#define ENTRYPOINT
#endif


#include <string>
#include <map>
#include <mutex>

#include "xprs.h"

#include "simpleapi/simpleApi.h"
#include "xpress_callback.h"


struct ASL;


namespace ampls
{

namespace xpress
{
  namespace impl
  {
    struct XPressDriverState;
    extern "C" {
      ENTRYPOINT void AMPLXPRESSfreeEnv();

      ENTRYPOINT XPressDriverState* AMPLXPRESSloadModel(int argc, char** argv,
        XPRSprob* modelPtr);

      ENTRYPOINT void AMPLXPRESSwriteSolution(XPressDriverState* state,
        XPRSprob modelPtr);

    }
    
    enum class XPRESSWhere
    {
      message,
      intsol,
      chgnode,
      infnode,
      nodecutoff,
      chgbranch,
      prenode,
      optnode
    };
    class CBWrap
    {
    public:
      static XPRESSCallback* setDefaultCB(XPRSprob prob, void* object, XPRESSWhere whereFrom);

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

      // Declares a node selection callback function. 
      // This is called every time the code backtracks to select a new node during the MIP search.
      // nodnum 
      // node: a pointer to the number of the node, nodnum, selected by the Optimizer.
      // By changing the value pointed to by this argument, the selected node may be changed with 
      // this function.
      static void XPRS_CC chgnode_callback_wrapper(XPRSprob prob, void* object,
        int* node);

      // Declares a user infeasible node callback function, 
      // called after the current node has been found to be infeasible during the Branch and Bound search.
      static void XPRS_CC infnode_callback_wrapper(XPRSprob prob, void* object);

      // Declares a user node cutoff callback function, 
      // called every time a node is cut off as a result of an improved integer solution being found 
      // during the Branch and Bound search.
      // node The number of the node that is cut off.
      static void XPRS_CC nodecutoff_callback_wrapper(XPRSprob prob, void* object, int node);

      // Declares a branching variable callback function, called every time a new branching variable 
      // is set or selected during the MIP search.
      // entity: A pointer to the variable or set on which to branch.Ordinary global variables are identified by their column index, i.e. 0, 1, ...(COLS - 1) and by their set index, i.e. 0, 1, ..., (SETS - 1).
      // up: If entity is a variable, this is 1 if the upward branch is to be made first, or 0 otherwise.If entity is a set, this is 3 if the upward branch is to be made first, or 2 otherwise.
      // estdeg: The estimated degradation at the node.
      static void XPRS_CC chgbranch_callback_wrapper(XPRSprob prob, void* vdata,
        int* entity, int* up, double* estdeg);

      // Declares a preprocess node callback function, called before the node has been optimized,
      // so the solution at the node will not be available.
      // feas: the feasibility status, if set to a nonzero value by the user, the current node 
      // will be declared infeasible by the optimizer.
      static void XPRS_CC prenode_callback_wrapper(XPRSprob prob, void* data, int* feas);

      // Declares an optimal node callback function, called after an optimal solution for the current 
      // node has been found during the Branch and Bound search.
      // feas: The feasibility status.If set to a nonzero value by the user, the current node will be declared infeasible.
      static void XPRS_CC optnode_callback_wrapper(XPRSprob my_prob, void* my_object, int* feas);
    };
  }
}

class XPRESSCallback;
class XPRESSModel;

/**
Encapsulates the main environment of the gurobi driver;
without modifications, a static CPLEXENV is created in the
AMPL driver, and it would be fairly easy to lose track of it;
this way, it is deleted in the destructor.
*/
class XPRESSDrv : public impl::SolverDriver<XPRESSModel> {
  void freeXPRESSEnv();
  XPRESSModel* loadModelImpl(char** args);
public:
  XPRESSModel loadModel(const char* modelName);
  ~XPRESSDrv();
};

/**
Encapsulates all the instance level information for a CPLEX model,
namely the CPLEX object, the relative ASL and all the locals of the
driver up to the moment in which optimize would be called.
It can not be created any other way than by reading an nl file,
and any assignment moves actual ownership.
At the end of its life, it deletes the relative structures.
*/
class XPRESSModel : public AMPLModel {
  friend XPRESSDrv;

  mutable bool copied_;
  xpress::impl::XPressDriverState* state_;
  XPRSprob prob_;

  XPRESSModel() :  copied_(false), state_(NULL),
    prob_(NULL) {}
  
  int setCallbackDerived(impl::BaseCallback* callback);
  impl::BaseCallback* createCallbackImplDerived(GenericCallback* callback);

public:

  XPRESSModel(const XPRESSModel& other) :
    AMPLModel(other),
    state_(other.state_),
    prob_(other.prob_),
    copied_(false)
  {
    fileName_ = other.fileName_;
    other.copied_ = true;
  }

  void writeSol();


  Status::Status getStatus() {
    throw AMPLSolverException("TBD");
  }

  int optimize();

  int getNumVars() {
    return getInt(XPRS_COLS);
  }
  double getObj() {
    if (getInt(XPRS_ALGORITHM) == 1) // no lp set
      return getDouble(XPRS_MIPOBJVAL);
    else
      return getDouble(XPRS_LPOBJVAL);
  }

  int getSolution(int first, int length, double* sol) {
    throw AMPLSolverException("TBD");
  }
  std::string error(int code);


  // XPRESS-specific
  // Access to XPRESS C structures
  XPRSprob getXPRSprob() {
    return prob_;
  }
 
  ~XPRESSModel() {
    if (copied_)
      return;
   
  }

  int getInt(int what) {
    int ret;
    XPRSgetintattrib(prob_, what, &ret);
    return ret;
  }
  int getDouble(int what) {
    double ret;
    XPRSgetdblattrib(prob_, what, &ret);
    return ret;
  }
};

} // namespace
#endif // CPLEX_INTERFACE_H_INCLUDE_