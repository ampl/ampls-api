#ifndef CBC_CALLBACK_H_INCLUDE_
#define CBC_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "Cbc_C_Interface.h"

namespace ampls {
namespace impl { namespace cbcmp{
void cut_callback_wrapper(void* osisolver, void* osicuts, void* appdata, int level, int pass);
void callback_wrapper(Cbc_Model* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
  int nchar, char** cvec);
} }
class CbcModel;

/**
* Base class for Cbc callbacks, inherit from this to declare a
* callback to be called at various stages of the solution process.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class CbcCallback : public impl::BaseCallback {
  friend void impl::cbcmp::cut_callback_wrapper(void* osisolver, void* osicuts, void* appdata, int level, int pass);
  friend void impl::cbcmp::callback_wrapper(Cbc_Model* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
    int nchar, char** cvec);
  friend class CbcModel;
  void* osisolver_;
  void* osicuts_;
  std::string msg_;
  static char toCBCSense(ampls::CutDirection::Direction dir)
  {
    switch (dir)
    {
    case CutDirection::EQ:
      return 'E';
    case CutDirection::GE:
      return 'G';
    case CutDirection::LE:
      return 'L';
    }
    throw std::runtime_error("Unexpected CutDirection value");
  }



protected:
  // Interface
  int doAddCut(const ampls::Constraint& c, int type);

public:
  void call_msg_callback(Cbc_Model* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
    int nchar, char** cvec) {
    where_ = ampls::Where::MSG;
    if (nchar > 0)
      msg_ = std::string(cvec[0])+ '\n';
    for (int i = 1; i < nchar; i++)
      msg_ += std::string(cvec[1]) + '\n';
    run();
  }

  CbcCallback() : osisolver_(NULL), osicuts_(NULL) {}
  
  virtual int run() = 0;
  /**
  Get a string description of where the callback was called from
  */
  const char* getWhereString();
  /**
  To get the cbc log message
  */
  const char* getMessage();

  using BaseCallback::getSolutionVector;
  using BaseCallback::getWhere;
  int getSolution(int len, double* sol);
  double getObj();

  // ************** Cbc specific **************
  /** Get osisolver, useful for calling cbc c library functions */
  void* getOSISolver() { return osisolver_; }
  /** Get osicuts, useful for calling cbc c library functions */
  void* getOSICuts() { return osicuts_; }
  /** * Get the underlying cbc model pointer */
  CbcModel* getCBCModel();
  /** Terminate the solution */
  void terminate();
  /** Get an integer attribute (using cbc C library enumeration to specify what)*/
  int getInt(int what) {
    int res;
    //TODO
  //  int status = GRBcbget(cbdata_, where_, what, &res);
  //  if (status)
   //   throw ampls::AMPLSolverException::format("Error while getting int attribute, code: %d", status);
    return 0;
  }
  /** Get a double attribute (using cbc C library enumeration to specify what)*/
  double getDouble(int what) {
    double res;
    //TODO
      //  int status = GRBcbget(cbdata_, where_, what, &res);
      //  if (status)
       //   throw ampls::AMPLSolverException::format("Error while getting int attribute, code: %d", status);
    return 0;
  }
  /** Get a double array attribute (using cbc C library enumeration to specify what)*/
  std::vector<double> getDoubleArray(int what) {
    int len = model_->getNumVars();
    //TODO
      //  int status = GRBcbget(cbdata_, where_, what, &res);
      //  if (status)
    //throw ampls::AMPLSolverException::format("Error while getting int attribute, code: %d", status);
    return std::vector<double>();
  }
  /** Set the current solution */
  double setSolution(double* x)
  {
    double obj;
    //TODO
      //  int status = GRBcbget(cbdata_, where_, what, &res);
      //  if (status)
       //   throw ampls::AMPLSolverException::format("Error while getting int attribute, code: %d", status);
    return 0;
  }

  virtual Where::CBWhere getAMPLSWhere() {
    return (ampls::Where::CBWhere)where_;
  }
  /** Get a value (using cbc C library enumeration to specify what)*/
  Variant get(int what);
  
  virtual Variant getValue(Value::CBValue v);

  int setHeuristicSolution(int nvars, const int* indices, const double* values);


  std::vector<double> getValueArray(Value::CBValue v)
  {
   /* switch (v)
    {
    case Value::MIP_SOL_RELAXED:
      if (where_ == GRB_CB_MIPNODE)
      {
        std::vector<double> res(model_->getNumVars());
        GRBcbget(cbdata_, where_, GRB_CB_MIPNODE_REL, res.data());
        return res;
      }
    }*/
    return std::vector<double>();
  }
};

} // namespace
#endif // CBC_CALLBACK_H_INCLUDE_
