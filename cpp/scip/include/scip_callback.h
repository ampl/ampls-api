#ifndef SCIP_CALLBACK_H_INCLUDE_
#define SCIP_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "scip/scip.h"

namespace ampls {
namespace impl { namespace scip{
void cut_callback_wrapper(void* solver, void* osicuts, void* appdata, int level, int pass);
void callback_wrapper(SCIP* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
  int nchar, char** cvec);
} }
class SCIPModel;

/**
* Base class for SCIP callbacks, inherit from this to declare a
* callback to be called at various stages of the solution process.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where the callback is called from, you can obtain various
* information about the progress of the optimization and can modify the behaviour
* of the solver.
*/
class SCIPCallback : public impl::BaseCallback {
  friend void impl::scip::cut_callback_wrapper(void* osisolver, void* osicuts, void* appdata, int level, int pass);
  friend void impl::scip::callback_wrapper(SCIP* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
    int nchar, char** cvec);
  friend class SCIPModel;
  void* model_;
  void* osicuts_;
  std::string msg_;
  static char toSCIPSense(ampls::CutDirection::Direction dir)
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
  void call_msg_callback(SCIP* model, int msgno, int ndouble, const double* dvec, int nint, const int* ivec,
    int nchar, char** cvec) {
    where_ = ampls::Where::MSG;
    if (nchar > 0)
      msg_ = std::string(cvec[0])+ '\n';
    for (int i = 1; i < nchar; i++)
      msg_ += std::string(cvec[1]) + '\n';
    run();
  }

  SCIPCallback() : model_(NULL), osicuts_(NULL) {}
  
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

  // ************** SCIP specific **************
  /** Get model, useful for calling SCIP c library functions */
  void* getModel() { return model_; }
  /** Get osicuts, useful for calling cbc c library functions */
  void* getOSICuts() { return osicuts_; }
  /** * Get the underlying SCIP model pointer */
  SCIP* getSCIPModel();
  /** Terminate the solution */
  void terminate();

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

  virtual Where::CBWhere getAMPLWhere() {
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
#endif // SCIP_CALLBACK_H_INCLUDE_
