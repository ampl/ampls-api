#ifndef SIMPLEAPI_H_INCLUDE_
#define SIMPLEAPI_H_INCLUDE_

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory> // for std::auto_ptr
#include <mutex>
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
  Variant(): str(NULL), integer(0), dbl(0), type(-1) {}
  Variant(const char* c) : str(c), integer(0), dbl(0), type(0) {}
  explicit Variant(int v) : str(NULL), integer(v), dbl(0), type(1) {}
  Variant(double v) : str(NULL), integer(0), dbl(v), type(2) {}

};

// Forward declarations
class AMPLModel;
class GenericCallback;
char** generateArguments(const char* modelName);
void deleteParams(char** params);

/**
* These (generic) values identify where in the solution
* process a callback has been called; to get this generic value
* call GenericCallback::getAMPLType().
* Not all solvers "where" are mapped to these values; in case
* the callback is called with a not-mapped "where" parameter,
* refer to the solver-specific functionality.
*/
enum class Where
{
  /** When the solver wants to print a message, obtain it via GenericCallback::getMessage.*/
  msg = 0, 
  /** Presolve phase */
  presolve = 1,
  /** Executing simplex */
  lpsolve = 2,
  /** Exploring a MIP node*/
  mipnode = 3,
  /** Found a new MIP solution*/
  mipsol = 4,
  /** Executing MIP algorithm*/
  mip = 5,
  /** Not mapped, refer to the specific user documentation*/
  notmapped = 10
};

/**
* Which (generic) values to get in a callback; just a subset of all the
* value types are mapped here.
* In case a not mapped value is required, refer to the solver-specific
* API.
*/
enum class Value {
  obj = 0,
  pre_delcols = 1,
  pre_delrows = 2,
  pre_coeffchanged = 3,
  iterations = 4,

  mip_relativegap = 5
};

/** Direction of a cut to be added*/
enum class CutDirection {
  /** = Equal*/
  eq,
  /** >= Greater or equal*/
  ge,
  /** <= Less or equal*/
  le
};

enum class Status {
  Unknown,
  Optimal,
  Infeasible,
  Unbounded,
  LimitIteration,
  LimitNode,
  LimitTime,
  LimitSolution,
  Interrupted,
  NotMapped
};
namespace impl
{
/**
* Infrastructure, should not be used directly.
* Base class for all callback objects, solvers-specific and/or generic.
*/
class BaseCallback
{
  friend class AMPLModel;
  friend class ampls::GenericCallback;

protected:
  AMPLModel* model_;
  int where_;
  virtual int doAddCut(int nvars, const int* vars,
    const double* coeffs, CutDirection direction, double rhs,
    int type) = 0;

  int callAddCut(std::vector<std::string>& vars,
    const double* coeffs, CutDirection direction, double rhs,
    int type);
  void printCut(int nvars, const int* vars, const double* coeffs, 
    CutDirection direction, double rhs)
  {
    char* sense;
    switch (direction)
    {
    case CutDirection::eq:
      sense = "= \0";
      break;
    case CutDirection::ge:
      sense = ">=\0";
      break;
    case CutDirection::le:
      sense = "<=\0";
      break;
    default:
      throw AMPLSolverException("Unexpected cut direction");
    }
    for (int i = 0; i < nvars; ++i) {
      printf("%f*x[%d]", coeffs[i], vars[i]);
      if (i < nvars - 1)
        printf(" + ");
    }
    printf(" %s %f\n", sense, rhs);

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

  /** Add a cut (AMPL variables names) 
  *
  *   @param direction Direction of the constraint ampls::CBDirection::Direction
  */
  int addCut(std::vector<std::string> vars,
    const double* coeffs, CutDirection direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 0);
  }
  /** Add a cut (solver indices) */
  int addCutsIndices(int nvars, const int* vars,
    const double* coeffs, CutDirection direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 0);
  }
  int addLazy(std::vector<std::string> vars,
    const double* coeffs, CutDirection direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 1);
  }
  int addLazyIndices(int nvars, const int* vars,
    const double* coeffs, CutDirection direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 1);
  }
  std::vector<double> getSolutionVector();
  /**
  Get the current solution
  */
  virtual int getSolution(int len, double* sol) = 0;

  virtual double getObjective() = 0;

  virtual const char* getWhere() = 0;

  virtual const char* getMessage() = 0;

  // Return mapped "whereFrom"
  // Obviously it only makes sense for the generic callback
  virtual Where getAMPLType() = 0;
  virtual Variant getValue(Value v) = 0;
};

/**
* Infrastructure, should not be used directly.
* Base class for the solver drivers.
*/
template<class T> class SolverDriver
{
  std::mutex loadMutex;
protected:
  virtual T* loadModelImpl(char** args) = 0;
public:
  
  std::auto_ptr<T> loadModelGeneric(const char* modelName)
  {
    FILE* f = fopen(modelName, "rb");
    if (!f)
      throw ampls::AMPLSolverException("Could not find file: " + std::string(modelName));
    else
      fclose(f);

    char** args = NULL;
    try {
      const std::lock_guard<std::mutex> lock(loadMutex);
      args = generateArguments(modelName);
      T* mod = loadModelImpl(args);
      deleteParams(args);
      return std::auto_ptr<T>(mod);
    }
    catch (const std::exception& e) {
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
* set it via AMPLModel::setGenericCallback() before starting the solution
* process via AMPLModel::solve().
* Depending on where the callback is called from, you can obtain various 
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class GenericCallback : public impl::BaseCallback
{
  friend class AMPLModel;

private:
  std::auto_ptr<impl::BaseCallback> impl_;

protected:
  virtual int doAddCut(int nvars, const int *vars,
                       const double *coeffs, CutDirection direction, double rhs,
                       int type)
  {
    return impl_->doAddCut(nvars, vars, coeffs, direction, rhs, type);
  }

public:
  /** Get the current solution vector */
  int getSolution(int len, double *sol)
  {
    return impl_->getSolution(len, sol);
  }
  /** Get the current objective value */
  double getObjective()
  {
    return impl_->getObjective();
  }
  /** Get a textual representation of the current solver status*/
  const char *getWhere()
  {
    return impl_->getWhere();
  }
  /** Get the message that was being printed (if where == msg) */
  const char *getMessage()
  {
    return impl_->getMessage();
  }
  /** Return mapped "whereFrom" */
  Where getAMPLType()
  {
    return impl_->getAMPLType();
  }
  /** Get a value from the solver */
  Variant getValue(Value v)
  {
    return impl_->getValue(v);
  }
};
/**
* Store an in-memory representation of an AMPL model, which 
* can be constructed by loading it from an NL file.
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
  //AMPLModel(const char* fileName) : fileName_(fileName) {}
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

public:
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
  virtual Status getStatus() {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /**
  Start the optimization process
  */
  virtual int optimize() {
    throw AMPLSolverException("Not implemented in base class!");
  };

  /**
  * Write the solution file
  */
  virtual void writeSol() {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /**
  Get "length" variables of the current problem in an array, starting at the specified
  position 
  */
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
  Enable adding lazy constraints via callbacks
  */
  virtual void enableLazyConstraints() { }
  /**
  Utility function: prints all variables to screen
  */
  void printModelVars(bool onlyNonZero);
};

} // namespace
#endif // SIMPLEAPI_H_INCLUDE_
