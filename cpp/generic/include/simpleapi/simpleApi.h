#ifndef SIMPLEAPI_H_INCLUDE_
#define SIMPLEAPI_H_INCLUDE_

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory> // for std::auto_ptr
#include <mutex>
#include <cstdarg>

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
typedef struct Variant
{
  const char* str; // type 0
  int integer;     // type 1
  double dbl;      // type 2
  int type;
} Variant;

// Forward declarations
class AMPLModel;
class GenericCallback;
char** generateArguments(const char* modelName);
void deleteParams(char** params);

namespace CBWhere {
/**
* These (generic) values identify where in the solution
* process a callback has been called; to get this generic value
* call GenericCallback::getAMPLType().
* Not all solvers "where" are mapped to these values; in case
* the callback is called with a not-mapped "where" parameter,
* refer to the solver-specific functionality.
*/
enum Where
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
}

namespace CBValue {
/**
* Which (generic) values to get in a callback; just a subset of all the
* value types are mapped here.
* In case a not mapped value is required, refer to the solver-specific
* API.
*/
enum Value {
  obj = 0,
  pre_delcols = 1,
  pre_delrows = 2,
  pre_coeffchanged = 3,
  iterations = 4
};
}

namespace CBDirection
{
/** Direction of a cut to be added*/
enum Direction {
  /** = Equal*/
  eq,
  /** >= Greater or equal*/
  ge,
  /** <= Less or equal*/
  le
};
}
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
    const double* coeffs, int direction, double rhs,
    int type) = 0;

  int callAddCut(std::vector<std::string>& vars,
    const double* coeffs, int direction, double rhs,
    int type);
  void printCut(int nvars, const int* vars, const double* coeffs, 
    int direction, double rhs)
  {
    char* sense;
    switch (direction)
    {
    case CBDirection::eq:
      sense = "= \0";
      break;
    case CBDirection::ge:
      sense = ">=\0";
      break;
    case CBDirection::le:
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
    const double* coeffs, int direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 0);
  }
  /** Add a cut (solver indices) */
  int addCutsIndices(int nvars, const int* vars,
    const double* coeffs, int direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 0);
  }
  int addLazy(std::vector<std::string> vars,
    const double* coeffs, int direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 1);
  }
  int addLazyIndices(int nvars, const int* vars,
    const double* coeffs, int direction, double rhs)
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
  virtual CBWhere::Where getAMPLType() = 0;
  virtual Variant getValue(CBValue::Value v) = 0;
};
} // namespace impl


template<class T> class SolverDriver
{
  std::mutex loadMutex;
protected:
  virtual void loadModelImpl(char** args, T* model) = 0;

public:
  
  std::auto_ptr<T> loadModelGeneric(const char* modelName, const char* solver)
  {
    const char** args = generateArguments(modelName);
    const std::lock_guard<std::mutex> lock(loadMutex);
    FILE* f = fopen(modelName, "rb");
    if (!f)
      throw ampls::AMPLSolverException("Could not find file: " + std::string(modelName));
    else
      fclose(f);
    return std::auto_ptr<T>(loadModelImpl(args));
  }

  ~SolverDriver() {}

};
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
                       const double *coeffs, int direction, double rhs,
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
  CBWhere::Where getAMPLType()
  {
    return impl_->getAMPLType();
  }
  /** Get a value from the solver */
  Variant getValue(CBValue::Value v)
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
  
  int setGenericCallback(GenericCallback *callback)
  {
    callback->model_ = this;
    impl::BaseCallback *realcb = createCallbackImplDerived(callback);
    callback->impl_.reset(realcb);
    return setCallback(callback->impl_.get());
  }
  int setCallback(impl::BaseCallback *callback)
  {
    callback->model_ = this;
    return setCallbackDerived(callback);
  }

  std::vector<double> getSolutionVector();

  // Interface - to be implemented in each solver
  virtual int getNumVars() {
    throw AMPLSolverException("Not implemented in base class!");
  };

  virtual int optimize() {
    throw AMPLSolverException("Not implemented in base class!");
  };

  virtual void writeSol() {
    throw AMPLSolverException("Not implemented in base class!");
  };

  virtual int getSolution(int first, int length, double *sol) {
    throw AMPLSolverException("Not implemented in base class!");
  };

  virtual double getObj() {
    throw AMPLSolverException("Not implemented in base class!");
  };

  virtual std::string error(int code) {
    throw AMPLSolverException("Not implemented in base class!");
  };

  virtual void enableLazyConstraints() { }

  void printModelVars(bool onlyNonZero);
};

} // namespace
#endif // SIMPLEAPI_H_INCLUDE_
