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


  
    
    
    struct CPLEX_CB_PARAMS {
      int local;
      int purgeable;
    };
  
  protected:
    int doAddCut(const ampls::Constraint& c, int type, void* additionalParams = nullptr);
    int getSolution(int len, double* sol);
    double getObj();
    Variant getValueImpl(Value::CBValue v, int threadid0);
    Variant getValueImpl(Value::CBValue v);

  public:

    using BaseCallback::getSolutionVector;

    const char* getMessage() { return msg_; }
    // Thread aware version
    bool canDo(CanDo::Functionality f, int threadid);
    bool canDo(CanDo::Functionality f) { return canDo(f, 0); }

    // Interface
    const char* getWhereString() {
      return getWhereString(0);
    }
    const char* getWhereString(int threadid);

    Where::CBWhere getAMPLWhereImpl() {
      return getAMPLWhereImpl(0);
    }
    Where::CBWhere getAMPLWhereImpl(int threadid) {
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

    Where::CBWhere getAMPLWhere(int threadid=0) {
      return getAMPLWhereImpl(threadid);
    }
    
    std::vector<double> getValueArray(Value::CBValue v, int threadid) {
      return std::vector<double>();
    }
    
    Variant getValue(Value::CBValue v) {
      return getValueImpl(v, 0);
    }

    Variant getValue(Value::CBValue v, int threadid0) {
      return getValueImpl(v, threadid0);
    }
    std::vector<double> getValueArray(Value::CBValue v);
    

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

    
    /** CPLEX only: add a user cut using AMPL variables names, additionaly specifying
    * if the cut is to be considered local or global.
    * @param vars Vector of AMPL variable names
    * @param coeffs Vector of cut coefficients
    * @param direction Direction of the constraint ampls::CBDirection::Direction
    * @param rhs Right hand side value
    */
    ampls::Constraint addCut(std::vector<std::string> vars,
      const double* coeffs, CutDirection::Direction direction, double rhs,
      int local)
    {
      CPLEX_CB_PARAMS p = { CPX_USECUT_FORCE, local };
      return callAddCut(vars, coeffs, direction, rhs, 0, &p);
    }
   

    /** CPLEX only: Add a user cut using solver indices, additionaly specifying
    * if the cut is to be considered local or global
    * @param nvars Number of variables in the cut (length of *vars)
    * @param vars Vector of variable indices (in the solvers representation)
    * @param coeffs Vector of cut coefficients
    * @param direction Direction of the constraint ampls::CBDirection::Direction
    * @param rhs Right hand side value
    */
    ampls::Constraint addCutIndices(int nvars, const int* vars,
      const double* coeffs, CutDirection::Direction direction, double rhs,
      int local)
    {
      CPLEX_CB_PARAMS p = { CPX_USECUT_FORCE, local };
      return callDoAddCut(nvars, vars, coeffs, direction, rhs, 0, &p);
    }

    /** CPLEX ONLY: Add a lazy constraint using AMPL variables names, additionaly specifying
    * if the constraint is to be considered local or global
   * @param vars Vector of AMPL variable names
   * @param coeffs Vector of cut coefficients
   * @param direction Direction of the constraint ampls::CBDirection::Direction
   * @param rhs Right hand side value
   */
    ampls::Constraint addLazy(std::vector<std::string> vars,
      const double* coeffs, CutDirection::Direction direction, double rhs,
      int local)
    {
      CPLEX_CB_PARAMS p = { CPX_USECUT_FORCE, local };
      return callAddCut(vars, coeffs, direction, rhs, 1, &p);
    }

  
    /** Add a lazy constraint using solver indices, , additionaly specifying
    * if the constraint is to be considered local or global
    * @param nvars Number of variables in the cut (length of *vars)
    * @param vars Vector of variable indices (in the solvers representation)
    * @param coeffs Vector of cut coefficients
    * @param direction Direction of the constraint ampls::CBDirection::Direction
    * @param rhs Right hand side value
    */
    ampls::Constraint addLazyIndices(int nvars, const int* vars,
      const double* coeffs, CutDirection::Direction direction, double rhs,
      int local)
    {
      CPLEX_CB_PARAMS p = { CPX_USECUT_FORCE, local };
      return callDoAddCut(nvars, vars, coeffs, direction, rhs, 1, &p);
    }
   


  };
} // namespace ampls
#endif // CPLEX_CALLBACK_H_INCLUDE_