#ifndef ampls_H_INCLUDE_
#define ampls_H_INCLUDE_

#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory> // for std::auto_ptr
#include <cstdarg>

// This declaration is used in the solver-specific implementations
// to import functions from the solver libraries
#ifdef _WIN32
#define ENTRYPOINT __declspec(dllimport)
#else
#define ENTRYPOINT
#endif


namespace ampls
{
/**
Wrapper for all ampl solver exceptions
*/
class AMPLSolverException : public std::runtime_error
{
public:
  AMPLSolverException(const char* msg) : std::runtime_error(msg) { }

  AMPLSolverException(std::string& msg) : std::runtime_error(msg) { }

  static AMPLSolverException format(const char* msg, ...)
  {
    char buffer[1000];
    va_list aptr;
    va_start(aptr, msg);
    vsprintf(buffer, msg, aptr);
    va_end(aptr);
    return AMPLSolverException(buffer);
  }
};

/**
* Stores a solver return value.
* Note that the strings are owned by the solver itself,
* no need to destroy them
*/
struct Variant
{
  const char* str; // type 0
  int integer;     // type 1
  double dbl;      // type 2
  int type;
  Variant() : str(NULL), integer(0), dbl(0), type(-1) {}
  Variant(const char* c) : str(c), integer(0), dbl(0), type(0) {}
  explicit Variant(int v) : str(NULL), integer(v), dbl(0), type(1) {}
  Variant(double v) : str(NULL), integer(0), dbl(v), type(2) {}

};

// Forward declarations
class AMPLModel;
class GenericCallback;
char** generateArguments(const char* modelName);
void deleteParams(char** params);


struct Where
{
  /**
  * These values (generic) identify where in the solution
  * process a callback has been called; to get this generic value
  * call GenericCallback::getAMPLType().
  * Not all solvers "where" are mapped to these values; in case
  * the callback is called with a not-mapped "where" parameter,
  * refer to the solver-specific functionality.
  */
  enum CBWhere
  {
    /** When the solver wants to print a message, obtain it via GenericCallback::getMessage.*/
    MSG = 0,
    /** Presolve phase */
    PRESOLVE = 1,
    /** Executing simplex */
    LPSOLVE = 2,
    /** Exploring a MIP node*/
    MIPNODE = 3,
    /** Found a new MIP solution*/
    MIPSOL = 4,
    /** Executing MIP algorithm*/
    MIP = 5,
    /** Not mapped, refer to the specific user documentation*/
    NOTMAPPED = 10
  };
};


struct Value
{
  /**
  * Which values (generic) to get in a callback; just a subset of all the
  * value types are mapped here.
  * In case a not mapped value is required, refer to the solver-specific
  * API.
  */
  enum  CBValue {
    OBJ = 0,
    PRE_DELCOLS = 1,
    PRE_DELROWS = 2,
    PRE_COEFFCHANGED = 3,
    ITERATIONS = 4,

    MIP_RELATIVEGAP = 5
  };
};
struct CutDirection {
  /** Direction of a cut to be added*/
  enum Direction {
    /** = Equal*/
    EQ,
    /** >= Greater or equal*/
    GE,
    /** <= Less or equal*/
    LE
  };
};

struct Status
{
  /**
  * Solution status (generic)
  */
  enum SolStatus {
    UNKNOWN,
    OPTIMAL,
    INFEASIBLE,
    UNBOUNDED,
    LIMIT_ITERATION,
    LIMIT_NODE,
    LIMIT_TIME,
    LIMIT_SOLUTION,
    INTERRUPTED,
    NOTMAPPED
  };
};
} // namespace ampls

// Mutex utility
#if defined(_MSC_VER)
#if _MSC_VER >=1700 // __cplusplus is not supported on windows compiler 
// unless we specify the /Zc compiler setting, so we resort to the _MSC_VER
// std::mutex was first implemented in VC 2012.
#define USECPP11MUTEX
#else
#define USEWINLOCK
#include <windows.h>
typedef SRWLOCK MUTEXIMPL;
#endif  
#else
#if __cplusplus >= 201103L && __STDC_HOSTED__ == 1 && __STDCPP_THREADS__ == 1
#define USECPP11MUTEX
#else
#define USEPTHREADMUTEX
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
typedef pthread_mutex_t  MUTEXIMPL;
#endif
#endif
#ifdef USECPP11MUTEX
#include <mutex>
typedef std::mutex MUTEXIMPL;
#endif
namespace ampls{
namespace impl
{
class AMPLMutex {
public:
  inline AMPLMutex();
  inline ~AMPLMutex();
  inline void Lock();    
  inline void Unlock();
private:
  MUTEXIMPL mutex_;
  // Catch the error of writing Mutex when intending MutexLock.
      AMPLMutex(AMPLMutex*){}
      AMPLMutex(const AMPLMutex&) {}
      AMPLMutex& operator=(const AMPLMutex& m);
    };
#ifdef USECPP11MUTEX
    AMPLMutex::AMPLMutex() { }
    AMPLMutex::~AMPLMutex() { }
    void AMPLMutex::Lock() { mutex_.lock(); }
    void AMPLMutex::Unlock() { mutex_.unlock(); }
#elif defined(USEWINLOCK)
    AMPLMutex::AMPLMutex() { InitializeSRWLock(&mutex_); }
    AMPLMutex::~AMPLMutex() { }
    void AMPLMutex::Lock() { AcquireSRWLockExclusive(&mutex_); }
    void AMPLMutex::Unlock() { ReleaseSRWLockExclusive(&mutex_); }
#elif defined(USEPTHREADMUTEX)
    AMPLMutex::AMPLMutex() { pthread_mutex_init(&mutex_, NULL); }
    AMPLMutex::~AMPLMutex() { pthread_mutex_destroy(&mutex_); }
    void AMPLMutex::Lock() { pthread_mutex_lock(&mutex_); }
    void AMPLMutex::Unlock() { pthread_mutex_unlock(&mutex_); }
#undef SAFE_PTHREAD

#endif

/**
* Infrastructure, should not be used directly.
* Base class for all callback objects, solvers-specific and/or generic.
*/
class BaseCallback
{
  friend class ampls::AMPLModel;
  friend class ampls::GenericCallback;

protected:
  AMPLModel* model_;
  int where_;
  virtual int doAddCut(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs,
    int type) = 0;

  int callAddCut(std::vector<std::string>& vars,
    const double* coeffs, CutDirection::Direction direction, double rhs,
    int type);
  void printCut(int nvars, const int* vars, const double* coeffs, 
    CutDirection::Direction direction, double rhs)
  {
    std::string sense;
    switch (direction)
    {
    case CutDirection::EQ:
      sense = "= ";
      break;
    case CutDirection::GE:
      sense = ">=";
      break;
    case CutDirection::LE:
      sense = "<=";
      break;
    default:
      throw AMPLSolverException("Unexpected cut direction");
    }
    for (int i = 0; i < nvars; ++i) {
      printf("%f*x[%d]", coeffs[i], vars[i]);
      if (i < nvars - 1)
        printf(" + ");
    }
    printf(" %s %f\n", sense.c_str(), rhs);

  }
public:
  BaseCallback() : model_(NULL) {}
  /** Function to override, called periodically by the optimizer*/
  virtual int run() = 0;
  /** Get the map AMPLEntityName -> SolverVarIndex*/
  std::map<std::string, int>& getVarMap();
  /** Get the map SolverVarIndex -> AMPLEntityName*/
  std::map<int, std::string>& getVarMapInverse();

  virtual ~BaseCallback() {};

  /** Add a user cut using AMPL variables names.
  * @param vars Vector of AMPL variable names
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  int addCut(std::vector<std::string> vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 0);
  }
    /** Add a lazy constraint using AMPL variables names.
  * @param vars Vector of AMPL variable names
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  int addLazy(std::vector<std::string> vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 1);
  }

  /** Add a user cut using solver indics
  * @param nvars Number of variables in the cut (length of *vars)
  * @param vars Vector of variable indices (in the solvers representation)
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  int addCutsIndices(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 0);
  }
  /** Add a lazy constraint using solver indics
  * @param nvars Number of variables in the cut (length of *vars)
  * @param vars Vector of variable indices (in the solvers representation)
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  int addLazyIndices(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 1);
  }
  /** Get the current solution vector */
  std::vector<double> getSolutionVector();
  /** Get the current solution */
  virtual int getSolution(int len, double* sol) = 0;
  /** Get the current objective value */
  virtual double getObj() = 0;
  /** Get an iteger representing where in the solution process the callback has been called.
     NOTE: this is expressed using the solver's own (not mapped) values
  */
  virtual int getWhere() { return where_; }
  /** Get a textual representation of where in the solution process the callback has been called.
   * NOTE: this is expressed using the solver's own (not mapped) values
   */
  virtual const char* getWhereString() = 0;
  
  /** Get the message from the solver (available only for specific values of getWhere) */
  virtual const char* getMessage() = 0;

  /** Get where in the solution process the callback has been called (mapped) */
  virtual Where::CBWhere getAMPLWhere() = 0;
  /** Get a (mapped) value */
  virtual Variant getValue(Value::CBValue v) = 0;
};

/**
* Infrastructure, should not be used directly.
* Base class for the solver drivers.
*/
template<class T> class SolverDriver
{
  AMPLMutex loadMutex;
protected:
  virtual T* loadModelImpl(char** args) = 0;
public:
  /**
  Not to be used directly; to be called in the solver driver `loadModel` function implementations to provide
  common functionalities like mutex and exception handling
  */
  T* loadModelGeneric(const char* modelName)
  {
    FILE* f = fopen(modelName, "rb");
    if (!f)
      throw ampls::AMPLSolverException::format("Could not find file: %s", modelName);
    else
      fclose(f);

    char** args = NULL;
    try {
      
      args = generateArguments(modelName);
      loadMutex.Lock();
      T* mod = loadModelImpl(args);
      loadMutex.Unlock();
      deleteParams(args);
      return mod;
    }
    catch (const std::exception& e) {
      loadMutex.Unlock();
      deleteParams(args);
      throw e;
    }
  }

  ~SolverDriver() {}

};
} // namespace impl

/**
* Base abstract class for generic callbacks, inherit from this to declare a 
* generic callback.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various 
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class GenericCallback : public impl::BaseCallback
{
  friend class AMPLModel;

private:
  std::unique_ptr<impl::BaseCallback> impl_;

protected:
  virtual int doAddCut(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs,
                       int type)
  {
    return impl_->doAddCut(nvars, vars, coeffs, direction, rhs, type);
  }

public:
  /** Get the current solution vector   */
  int getSolution(int len, double *sol)
  {
    return impl_->getSolution(len, sol);
  }
  /** Get the current objective value */
  double getObj()
  {
    return impl_->getObj();
  }
  /** Get an iteger representing where in the solution process the callback has been called.
   * NOTE: this is expressed using the solver's own (not mapped) values
  */
  int getWhere()
  {
    return impl_->getWhere();
  }
  /** Get where in the solution process the callback has been called (mapped) */
  Where::CBWhere getAMPLWhere()
  {
    return impl_->getAMPLWhere();
  }
  /** Get a textual representation of the current solver status*/
  const char *getWhereString()
  {
    return impl_->getWhereString();
  }
  /** Get the message that was being printed (if where == msg) */
  const char *getMessage()
  {
    return impl_->getMessage();
  }
  /** Get a value from the solver */
  Variant getValue(Value::CBValue v)
  {
    return impl_->getValue(v);
  }
};
  
/**
* Store an in-memory representation of an AMPL model, which 
* can be constructed by loading it from an NL file using the `loadModel` function
* available in a solver driver (i.e. CPLEXDrv::loadModel(), 
* GurobiDrv::loadModel() or XPRESSDrv::loadModel()).
* It also contains two-way mappings between solver column and row numbers and
* AMPL entity names.
*/
class AMPLModel
{
  friend std::map<std::string, int>& impl::BaseCallback::getVarMap();
  friend std::map<int, std::string>& impl::BaseCallback::getVarMapInverse();
  std::map<int, std::string> varMapInverse_;
  std::map<std::string, int> varMap_;
  /*
  Create a cache of the names to indices maps, to be used
  in subsequent calls to a callback
  */
  void getVarMapsInternal()
  {
    if (varMap_.size() == 0)
      varMap_ = getVarMapFiltered(NULL);
    if (varMapInverse_.size() == 0)
      varMapInverse_ = getVarMapInverse();
  }

protected:
  std::string fileName_;
  AMPLModel() {}
  void resetVarMapInternal()
  {
    // Clear the internally cached maps
    varMap_.clear();
    varMapInverse_.clear();
  }
  virtual int setCallbackDerived(impl::BaseCallback* callback) {
    throw AMPLSolverException("Not implemented in base class!");
  };
  virtual impl::BaseCallback *createCallbackImplDerived(GenericCallback *callback)   {
    throw AMPLSolverException("Not implemented in base class!");
  };
  virtual void writeSolImpl(const char* solFileName) {
    throw AMPLSolverException("Not implemented in base class!");
  };
public:
  /*
  Get the name of the NL file where the model has been loaded from
  */
  std::string getFileName() {
    return fileName_;
  }
  /*
  Copy constructor
  */
  AMPLModel(const AMPLModel &other) : fileName_(other.fileName_) {}

  /**
  Get the map from variable name to index in the solver interface
  */
  std::map<int, std::string> getVarMapInverse();

  /**
  Get the map from variable name to index in the solver interface
  */
  std::map<std::string, int> getVarMap()
  {
    return getVarMapFiltered(NULL);
  }
  /**
  Return the variable map, filtered by the variable name
  */
  std::map<std::string, int> getVarMapFiltered(const char *beginWith);
  
  /**
  Set a generic callback to be called during optimization. This function is
  automatically dispatched when (and only when) assigning an ampls::GenericCallback,
  as it needs a special treatment to automatically create the solver-specific wrapper
  */
  int setCallback(GenericCallback *callback)
  {
    callback->model_ = this;
    impl::BaseCallback *realcb = createCallbackImplDerived(callback);
    callback->impl_.reset(realcb);
    return setCallback(callback->impl_.get());
  }
  /**
  Set callback to be called during optimization
  */
  int setCallback(impl::BaseCallback *callback)
  {
    callback->model_ = this;
    return setCallbackDerived(callback);
  }
  /**
  Get all the variables of the current problem
  */
  std::vector<double> getSolutionVector();
  /**
  Get the number of variables
  */
  virtual int getNumVars() {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /** 
  Get the solution status
  */
  virtual Status::SolStatus getStatus() {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /**
  Start the optimization process
  */
  virtual int optimize() {
    throw AMPLSolverException("Not implemented in base class!");
  };

  /**
  Write the solution file to the defualt location (filename.sol in the original directory)
  */
  virtual void writeSol() {
    writeSolImpl(NULL);
  };
  /**
  Write the solution file to a specific file
  */
  virtual void writeSol(const char* solFileName) {
    writeSolImpl(solFileName);
  };
  /**
  Get "length" variables of the current problem in an array, starting at the specified
  position */
  virtual int getSolution(int first, int length, double *sol) {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /**
  Get the current objective value
  */
  virtual double getObj() {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /**
  Get the error message corresponding to the code
  */
  virtual std::string error(int code) {
    throw AMPLSolverException("Not implemented in base class!");
  };

  /**
  Enable adding lazy constraints via callbacks (to be called only once)
  */
  virtual void enableLazyConstraints() { }
  /**
  Utility function: prints all variables to screen
  */
  void printModelVars(bool onlyNonZero);
};

} // namespace
#endif // ampls_H_INCLUDE_
