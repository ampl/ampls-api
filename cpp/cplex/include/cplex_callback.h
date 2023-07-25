#ifndef CPLEX_CALLBACK_H_INCLUDE_
#define CPLEX_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"
#include "ilcplex/cplex.h"

#define CPX_ENUM_MSG_CALLBACK -1

namespace ampls
{
  namespace impl { namespace cpx { class CBWrap; } }
  class CPLEXModel;

  /**
  * Base class for CPLEX callbacks, inherit from this to declare a
  * callback to be called at various stages of the solution process.
  * Provides all mapping between solver-specific and generic values.
  * To implement a callback, you should implement the run() method and
  * set it via AMPLModel::setCallback() before starting the solution
  * process via AMPLModel::optimize().
  * Depending on where the callback is called from, you can obtain various
  * information about the progress of the optimization and can modify the behaviour
  * of the solver.
  */
  class CPLEXCallback : public impl::BaseCallback {
    char CODE[60];
    // Stores the message passed from CPLEX msg function to the callback
    const char* msg_;
    // Thread-local data for multithreaded callbacks
    struct LOCALDATA {
      int where_;
      CPXCALLBACKCONTEXTptr context_;
    };
    std::vector<LOCALDATA> local_;
    bool hasThreads_;

    friend class CPLEXModel;
    friend class  impl::cpx::CBWrap;

    static char toCPLEXSense(CutDirection::Direction direction);

    CPXCALLBACKCONTEXTptr context(int threadId) { 
      return local_[threadId].context_; }


    // Interface
    int doAddCut(const ampls::Constraint& c, int type);
    
  protected:
    const char* getMessage() { return msg_; }
    // Thread aware version
    bool canDo(CanDo::Functionality f, int threadid);
    bool canDo(CanDo::Functionality f) { return canDo(f, 0); }

    // Interface
    using BaseCallback::getSolutionVector;
    int getSolution(int len, double* sol);
    double getObj();

    
    const char* getWhereString() {
      return getWhereString(0);
    }
    const char* getWhereString(int threadid);

    Where::CBWhere getAMPLWhere() {
      return getAMPLWhere(0);
    }
    Where::CBWhere getAMPLWhere(int threadid) {
      switch (getWhere(threadid))
      {
      case CPX_ENUM_MSG_CALLBACK: return Where::MSG;
      case CPX_CALLBACKCONTEXT_CANDIDATE: return Where::MIPSOL;
      case CPX_CALLBACKCONTEXT_RELAXATION: return Where::LPSOLVE;
      case CPX_CALLBACKCONTEXT_LOCAL_PROGRESS: return Where::MIPNODE;
      case CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS: return Where::MIPNODE;
      default:
        return Where::NOTMAPPED;
      }
    }

    Variant getValueImpl(Value::CBValue v);
    Variant getValueImpl(Value::CBValue v, int threadid0);

    Variant getValue(Value::CBValue v, int threadid0) {
      return getValueImpl(v, threadid0);
    }
    std::vector<double> getValueArray(Value::CBValue v);
    std::vector<double> getValueArray(Value::CBValue v, int threadid);

    int setHeuristicSolution(int nvars, const int* indices, const double* values);

    /** Get a  value at this stage of the solution process */
    Variant getCPLEXInfo(CPXCALLBACKINFO what, int threadid = 0);
    /** Get an integer value at this stage of the solution process*/
    int getCPLEXInt(CPXCALLBACKINFO what, int threadid = 0);
    /** Get a double value at this stage of the solution process*/
    double getCPLEXDouble(CPXCALLBACKINFO what, int threadid = 0);
    /** Get a double value at this stage of the solution process*/
    long getCPLEXLong(CPXCALLBACKINFO what, int threadid = 0);
    /** Get access to CPLEX callback native context */
    CPXCALLBACKCONTEXTptr getCPXContext(int threadId = 0) {
      return local_[threadId].context_;
    }

  public:
    int nnodes;
    CPLEXCallback() : CODE(""), BaseCallback(), msg_(nullptr),
      local_(1), hasThreads_(false), nnodes(0) { }

    /// <summary>
    ///  CPLEX specific. Usable only after linking the callback to a model 
    /// using AMPLModel::setCallback.
    /// Returns the number of cores in the system or the value of
    /// the options threads, if present in the model
    /// </summary>
    int getMaxThreads();
    
    /// <summary>
    /// Enable threads support for this callback.
    /// Maximum supported threads = ncores if nthreads==0,
    /// else nthreads
    /// </summary>
    void enableThreadsSupport(int nthreads=0) {
      if (nthreads < 0)
        throw std::out_of_range("Number of threads cannot be negative");
      local_.resize(nthreads);
      hasThreads_ = true;
    }
    /// <summary>
    /// Get CPLEX callback context id
    /// </summary>
    int getWhere(int threadid = 0) {
      return local_[threadid].where_;
    }
    /// <summary>
    /// Override to implement generic (or single threaded) callback 
    /// </summary>
    virtual int run() { 
      throw ampls::AMPLSolverException("If support for multithreading is not needed, "
        "the function void run() should be implemented"); }

    /// <summary>
    /// Override to implement multi-threaded callback (CPLEX-specific)
    /// </summary>
    virtual int run(int threadid) {
      throw ampls::AMPLSolverException("If support for multithreading is needed, "
        "the function void run(int) should be implemented");
    }
  };
} // namespace ampls
#endif // CPLEX_CALLBACK_H_INCLUDE_