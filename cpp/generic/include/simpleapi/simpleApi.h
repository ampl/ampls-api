#ifndef SIMPLEAPI_H_INCLUDE_
#define SIMPLEAPI_H_INCLUDE_

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory> // for std::auto_ptr

namespace ampl 
{

class AMPLSolverException : public std::runtime_error
{
public:
  AMPLSolverException(const char* msg) : std::runtime_error(msg)
  {
  }
  AMPLSolverException(std::string &msg) : std::runtime_error(msg)
  {
  }
};

typedef struct myobj
{
  const char *str; // type 0
  int integer;     // type 1
  double dbl;      // type 2
  int type;
} myobj;

class AMPLModel;

char **generateArguments(const char *modelName);
void deleteParams(char **params);

namespace AMPLCBWhere
{
enum Where
{
  msg = 0,
  presolve = 1,
  lpsolve = 2,
  mipnode = 3,
  mipsol = 4,
  mip =5,
  notmapped = 10
};
}
namespace AMPLCBValue
{
enum Value {
  obj = 0,
  pre_delcols = 1,
  pre_delrows = 2,
  pre_coeffchanged = 3,
  iterations = 4
};
}
class BaseCallback
{
  friend class AMPLModel;
  friend class GenericCallback;

protected:
  AMPLModel *model_;
  virtual int doAddCut(int nvars, const int *vars,
                       const double *coeffs, char direction, double rhs,
                       int type) = 0;

  int callAddCut(std::vector<std::string> &vars,
                 const double *coeffs, char direction, double rhs,
                 int type);

public:
  BaseCallback() : model_(NULL) {}

  virtual int run(int whereFrom) = 0;

  std::map<std::string, int> &getVarMap();
  std::map<int, std::string>& getVarMapInverse();
  
  virtual ~BaseCallback(){};

  /**
Direction: GRB_LESS_EQUAL, GRB_EQUAL, or GRB_GREATER_EQUAL
*/
  int addCut(std::vector<std::string> vars,
             const double *coeffs, char direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 0);
  }
  int addCutsIndices(int nvars, const int *vars,
                     const double *coeffs, char direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 0);
  }
  int addLazy(std::vector<std::string> vars,
              const double *coeffs, char direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 1);
  }
  int addLazyIndices(int nvars, const int *vars,
                     const double *coeffs, char direction, double rhs)
  {
    return doAddCut(nvars, vars, coeffs, direction, rhs, 1);
  }
  /*
  Note that this is allocated on the heap and must be destoyed (same in the Gurobi C++ library).
  It is quite nice to have it, as the python swig wrapper takes care of it automatically,
  but we will definitely get rid of it if we want to expose the C++ interface, as we'll have to do PIMPL
  */
  double *getSolutionVector(int *len);
  /**
  Get the current solution
  */
  virtual int getSolution(int len, double *sol) = 0;

  virtual double getObjective() = 0;

  virtual const char *getWhere(int where) = 0;

  virtual const char *getMessage() = 0;

  // Return mapped "whereFrom"
  // So far only return 1 if from msg callback
  // Can decide to implement proper mapping for most important
  // features later
  // Obviously it only makes sense for the generic callback
  virtual AMPLCBWhere::Where getAMPLType() = 0;
  virtual myobj getValue(AMPLCBValue::Value v) = 0;
};

class GenericCallback : public BaseCallback
{
  friend class AMPLModel;

private:
  std::auto_ptr<BaseCallback> impl_;

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

class AMPLModel
{
  friend std::map<std::string, int> &BaseCallback::getVarMap();
  friend std::map<int, std::string>& BaseCallback::getVarMapInverse();
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
  virtual int setCallbackDerived(BaseCallback *callback) = 0;
  virtual BaseCallback *createCallbackImplDerived(GenericCallback *callback) = 0;

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
    BaseCallback *realcb = createCallbackImplDerived(callback);
    callback->impl_.reset(realcb);
    return setCallback(callback->impl_.get());
  }
  int setCallback(BaseCallback *callback)
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
