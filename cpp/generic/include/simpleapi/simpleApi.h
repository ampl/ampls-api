#ifndef SIMPLEAPI_H_INCLUDE_
#define SIMPLEAPI_H_INCLUDE_

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory> // for std::auto_ptr

namespace ampl
{
/**
Wrapper for all ampl solver exceptions
*/
class AMPLSolverException : public std::runtime_error
{
public:
  AMPLSolverException(const char* msg) : std::runtime_error(msg)
  {
  }
  AMPLSolverException(std::string& msg) : std::runtime_error(msg)
  {
  }
};

/**
* Temp struct to store arbitrary data. Has ownership, dangerous in case of strings,
* useful for testing SWIG wrappers
* TODO reimplement
*/
typedef struct myobj
{
  const char* str; // type 0
  int integer;     // type 1
  double dbl;      // type 2
  int type;
} myobj;

// Forward declarations
class AMPLModel;
class GenericCallback;
char** generateArguments(const char* modelName);
void deleteParams(char** params);

namespace AMPLCBWhere
{
/**
* These (generic) values identify where in the solution
* process a callback has been called.
* Not all solvers "where" are mapped to these values; in case
* the callback is called with a not-mapped "where" parameter,
* refer to the solver-specific functionality.
*/
enum Where
{
  msg = 0,
  presolve = 1,
  lpsolve = 2,
  mipnode = 3,
  mipsol = 4,
  mip = 5,
  notmapped = 10
};
}
namespace AMPLCBValue
{
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

namespace impl
{
/**
* Infrastructure, should not be used directly.
* Base class for all callback objects, solvers-specific and/or generic.
*/
class BaseCallback
{
  friend class AMPLModel;
  friend class ampl::GenericCallback;

protected:
  AMPLModel* model_;
  virtual int doAddCut(int nvars, const int* vars,
    const double* coeffs, char direction, double rhs,
    int type) = 0;

  int callAddCut(std::vector<std::string>& vars,
    const double* coeffs, char direction, double rhs,
    int type);

public:
  BaseCallback() : model_(NULL) {}

  virtual int run(int whereFrom) = 0;

  std::map<std::string, int>& getVarMap();
  std::map<int, std::string>& getVarMapInverse();

  virtual ~BaseCallback() {};

  /**
Direction: GRB_LESS_EQUAL, GRB_EQUAL, or GRB_GREATER_EQUAL
*/
  int addCut(std::vector<std::string> vars,
    const double* coeffs, char direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 0);
  }
  int addCutsIndices(int nvars, const int* vars,
    const double* coeffs, char direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 0);
  }
  int addLazy(std::vector<std::string> vars,
    const double* coeffs, char direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 1);
  }
  int addLazyIndices(int nvars, const int* vars,
    const double* coeffs, char direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 1);
  }
  /*
  Note that this is allocated on the heap and must be destoyed (same in the Gurobi C++ library).
  It is quite nice to have it, as the python swig wrapper takes care of it automatically,
  but we will definitely get rid of it if we want to expose the C++ interface, as we'll have to do PIMPL
  */
  double* getSolutionVector(int* len);
  /**
  Get the current solution
  */
  virtual int getSolution(int len, double* sol) = 0;

  virtual double getObjective() = 0;

  virtual const char* getWhere(int where) = 0;

  virtual const char* getMessage() = 0;

  // Return mapped "whereFrom"
  // So far only return 1 if from msg callback
  // Can decide to implement proper mapping for most important
  // features later
  // Obviously it only makes sense for the generic callback
  virtual AMPLCBWhere::Where getAMPLType() = 0;
  virtual myobj getValue(AMPLCBValue::Value v) = 0;
};
} // namespace impl

/**
* Base class for generic callbacks, inherit from this to declare a 
* generic callback.
* Provides all mapping between solver-specific and generic values.
*/
class GenericCallback : public impl::BaseCallback
{
  friend class AMPLModel;

private:
  std::auto_ptr<impl::BaseCallback> impl_;

protected:
  virtual int doAddCut(int nvars, const int *vars,
                       const double *coeffs, char direction, double rhs,
                       int type)
  {
    return impl_->doAddCut(nvars, vars, coeffs, direction, rhs, type);
  }

public:
  // Interface
  int getSolution(int len, double *sol)
  {
    return impl_->getSolution(len, sol);
  }
  double getObjective()
  {
    return impl_->getObjective();
  }
  const char *getWhere(int wherefrom)
  {
    return impl_->getWhere(wherefrom);
  }
  const char *getMessage()
  {
    return impl_->getMessage();
  }
  // Return mapped "whereFrom"
  AMPLCBWhere::Where getAMPLType()
  {
    return impl_->getAMPLType();
  }
  myobj getValue(AMPLCBValue::Value v)
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
  virtual int setCallbackDerived(impl::BaseCallback *callback) = 0;
  virtual impl::BaseCallback *createCallbackImplDerived(GenericCallback *callback) = 0;

public:
  AMPLModel(const AMPLModel &other) : fileName_(other.fileName_) {}

  /*
  Get the map from variable name to index in the solver interface
  */
  std::map<int, std::string> getVarMapInverse();

  /*
  Get the map from variable name to index in the solver interface
  */
  std::map<std::string, int> getVarMap()
  {
    return getVarMapFiltered(NULL);
  }
  /*
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

  /*
  Note that this is allocated on the heap and must be destroyed (same in the Gurobi C++ library).
  It is quite nice to have it, as the python swig wrapper takes care of it automatically,
  but we will definitely get rid of it if we want to expose the C++ interface, as we'll have to do PIMPL
  */
  double *getSolutionVector(int *len);

  // Interface - to be implemented in each solver
  virtual int getNumVars() = 0;

  virtual int optimize() = 0;

  virtual void writeSol() = 0;

  virtual int getSolution(int first, int length, double *sol) = 0;

  virtual double getObj() = 0;

  virtual std::string error(int code) = 0;
};

} // namespace
#endif // SIMPLEAPI_H_INCLUDE_
