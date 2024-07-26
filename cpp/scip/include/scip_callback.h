#ifndef SCIP_CALLBACK_H_INCLUDE_
#define SCIP_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "scip/scip.h"

#include "scip/scipdefplugins.h"
#include "objscip/objscip.h"

namespace ampls {
  class SCIPCallback;
}
#ifdef __cplusplus
extern "C" {
  namespace ampls {
    namespace impl {
      namespace scip {
        namespace heur {
#endif
          SCIP_RETCODE heurExecXyz(SCIP* scip, SCIP_HEUR* heur, SCIP_HEURTIMING heurtiming,
            SCIP_Bool nodeinfeasible, SCIP_RESULT* result);
          SCIP_RETCODE SCIPincludeHeurXyz(
            SCIP* scip, /**< SCIP data structure */
            ampls::SCIPCallback* cb/**< SCIP data structure */
          );

#ifdef __cplusplus
        }
      }
    }
  }
}
#endif


namespace ampls {
  class SCIPCH;
  class SCIPModel;

  enum SCIPWhere {
    ConstraintHandler,
    Heuristics
  };


  class SCIPCallback : public impl::BaseCallback {
    friend class SCIPCH;
    friend class SCIPModel;
    friend SCIP_RETCODE impl::scip::heur::heurExecXyz(SCIP* scip, SCIP_HEUR* heur, SCIP_HEURTIMING heurtiming, 
      SCIP_Bool nodeinfeasible, SCIP_RESULT* result);
    SCIP* scip_;
    std::string whereString_;
    std::shared_ptr<SCIPCH> ch_;


    int actions_;
    struct PrimalSolution {
      std::vector<double> vars;
      std::vector<int> indices;
    };

    PrimalSolution solutionToSet_;

    void setWhere(SCIPWhere where, const char* str) {
      where_ = where;
      whereString_ = str;
    }
    void registerPlugins(SCIP* scip);

    std::vector<ampls::Constraint> cachedCons_;
    std::vector<int> pendingConstraints_;

    void addConstraintToScip(const ampls::Constraint& c, int type);
    bool hasPendingConstraints() {
      return pendingConstraints_.size() > 0;
    }
    void addPendingConstraints();
    int doAddCut(const ampls::Constraint& c, int type, 
    void* additionalParams=nullptr);

    SCIP_VAR** modelToScip(); 
    int getActions() { return actions_; }
  public:

    virtual int run() = 0;

    

    int setHeuristicSolution(int nvars, const int* indices, const double* values) {
      solutionToSet_.vars.insert(solutionToSet_.vars.end(), &values[0], &values[nvars]);
      solutionToSet_.indices.insert(solutionToSet_.indices.end(), &indices[0], &indices[nvars]);
      actions_ = 4;
      return 0;
    }

    int getSolution(int len, double* sol);

    double getObj(); 
    const char* getWhereString() { return whereString_.c_str() ;}
    const char* getMessage() { throw AMPLSolverException("Not implemented in SCIP!"); }

    ampls::Where::CBWhere getAMPLWhere() {
      switch (where_)
      {
        case SCIPWhere::ConstraintHandler:
          return ampls::Where::MIPSOL;
      }
      return ampls::Where::NOTMAPPED;
    }
    ampls::Variant getValueImpl(ampls::Value::CBValue v);

    std::vector<double> getValueArray(ampls::Value::CBValue v) { throw AMPLSolverException("Not implemented in SCIP!"); }

    SCIP* getSCIP() { return scip_; }
  };


class SCIPPlugin {
  SCIP* scip_;
  SCIPCallback* parent_;
public:
  SCIPPlugin(SCIP* scip, SCIPCallback* cb) : scip_(scip), parent_(cb) {}
  SCIP* getSCIP() {
    return scip_; }

  SCIPCallback* getCB() { return parent_; }
};



class SCIPPresol : public SCIPPlugin, scip::ObjPresol {
  friend class SCIPModel;
public:
  SCIPPresol(      
    SCIP*              model,              /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
    int                maxrounds,          /**< maximal number of presolving rounds the presolver participates in (-1: no limit) */
    SCIP_PRESOLTIMING  timing,              /**< timing mask of the presolver */
    SCIPCallback *cb) 
    : SCIPPlugin(model, cb), scip::ObjPresol(model, name, desc, priority, maxrounds, timing) {}

  private:
    SCIP_DECL_PRESOLFREE(scip_free) { return presol_free(); }
    SCIP_DECL_PRESOLINIT(scip_init) { return presol_init(); }
    SCIP_DECL_PRESOLEXIT(scip_exit) { return presol_exit(); }
    SCIP_DECL_PRESOLINITPRE(scip_initpre) { return presol_initpre(); }
    SCIP_DECL_PRESOLEXITPRE(scip_exitpre) { return presol_exitpre(); }
    SCIP_DECL_PRESOLEXEC(scip_exec) {
      return presol_exec(nrounds, presoltiming, nnewfixedvars, nnewaggrvars, nnewchgvartypes, nnewchgbds, nnewholes,
      nnewdelconss, nnewaddconss, nnewupgdconss, nnewchgcoefs, nnewchgsides, nfixedvars, naggrvars, nchgvartypes,
      nchgbds, naddholes, ndelconss, naddconss, nupgdconss, nchgcoefs, nchgsides, result);
    }

  public:
    virtual SCIP_RETCODE presol_free() { return SCIP_OKAY; }
    virtual SCIP_RETCODE presol_init() { return SCIP_OKAY; }
    virtual SCIP_RETCODE presol_exit() { return SCIP_OKAY; }
    virtual SCIP_RETCODE presol_initpre() { return SCIP_OKAY; }
    virtual SCIP_RETCODE presol_exitpre() { return SCIP_OKAY; }
    virtual SCIP_RETCODE presol_exec(int nrounds, SCIP_PRESOLTIMING presoltiming,
      int nnewfixedvars, int nnewaggrvars, int nnewchgvartypes, int nnewchgbds, int nnewholes,
      int nnewdelconss, int nnewaddconss, int nnewupgdconss, int nnewchgcoefs, int nnewchgsides,
      int* nfixedvars, int* naggrvars, int* nchgvartypes, int* nchgbds, int* naddholes,
      int* ndelconss, int* naddconss, int* nupgdconss, int* nchgcoefs, int* nchgsides, SCIP_RESULT* result) = 0;
};


class SCIPCH : public SCIPPlugin, public scip::ObjConshdlr {
  friend class SCIPModel;
  SCIP_SOL* sol_; // if in check

  public:
    SCIPCH(SCIP* model, SCIPCallback* cb) : SCIPPlugin(model, cb),
      scip::ObjConshdlr(model, "DEF", "DEF", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, SCIP_PRESOLTIMING_FAST)
    {}

    SCIPCH(
      SCIP* model,               /**< SCIP data structure */
      const char* name,               /**< name of constraint handler */
      const char* desc,               /**< description of constraint handler */
      int                sepapriority,       /**< priority of the constraint handler for separation */
      int                enfopriority,       /**< priority of the constraint handler for constraint enforcing */
      int                checkpriority,      /**< priority of the constraint handler for checking infeasibility (and propagation) */
      int                sepafreq,           /**< frequency for separating cuts; zero means to separate only in the root node */
      int                propfreq,           /**< frequency for propagating domains; zero means only preprocessing propagation */
      int                eagerfreq,          /**< frequency for using all instead of only the useful constraints in separation,
                                              *   propagation and enforcement, -1 for no eager evaluations, 0 for first only */
      int                maxprerounds,       /**< maximal number of presolving rounds the constraint handler participates in (-1: no limit) */
      SCIP_Bool          delaysepa,          /**< should separation method be delayed, if other separators found cuts? */
      SCIP_Bool          delayprop,          /**< should propagation method be delayed, if other propagators found reductions? */
      SCIP_Bool          needscons,          /**< should the constraint handler be skipped, if no constraints are available? */
      SCIP_PROPTIMING    proptiming,         /**< positions in the node solving loop where propagation method of constraint handlers should be executed */
      SCIP_PRESOLTIMING  presoltiming,
      SCIPCallback* cb) 
      : SCIPPlugin(model, cb), scip::ObjConshdlr(model, name, desc,
        sepapriority, enfopriority, checkpriority, sepafreq, propfreq, eagerfreq, maxprerounds,
        delaysepa, delayprop, needscons, proptiming, presoltiming)
    {

    }
    int getSolution(int len, double* sol, SCIP_VAR** mtos) {
      SCIP_VAR* var;
      for (auto i = 0; i < len; ++i)
      {
        SCIPgetTransformedVar(getSCIP(), mtos[i], &var);
        sol[i] = SCIPgetSolVal(getSCIP(), sol_, var);
      }
      return 0;
    }
      /** constraint enforcing method of constraint handler for pseudo solutions
   *
   *  @see SCIP_DECL_CONSENFOPS(x) in @ref type_cons.h
   */
  virtual SCIP_DECL_CONSENFOPS(scip_enfops) {
    getCB()->setWhere(SCIPWhere::ConstraintHandler, "CONSENFOPS");
    getCB()->currentCapabilities_ = CanDo::ADD_LAZY_CONSTRAINT | CanDo::GET_LP_SOLUTION | CanDo::GET_MIP_SOLUTION;
    sol_ = nullptr;
    getCB()->run();
    if (getCB()->getActions() == 1)
      *result = SCIP_CONSADDED;
    return SCIP_OKAY;
  }
  /** feasibility check method of constraint handler for primal solutions
   *
   *  @see SCIP_DECL_CONSCHECK(x) in @ref type_cons.h
   */
  virtual SCIP_DECL_CONSCHECK(scip_check) {
    getCB()->actions_ = 0;
    getCB()->setWhere(SCIPWhere::ConstraintHandler, "CONSCHECK");
    getCB()->currentCapabilities_ = CanDo::ADD_LAZY_CONSTRAINT | CanDo::GET_LP_SOLUTION | CanDo::GET_MIP_SOLUTION;
    sol_ = sol;
    getCB()->run();

    if (getCB()->getActions() != 0) *result = SCIP_INFEASIBLE;
    else *result= SCIP_FEASIBLE;
    return SCIP_OKAY;
  }

  /** variable rounding lock method of constraint handler
 *
 *  @see SCIP_DECL_CONSLOCK(x) in @ref type_cons.h
 */
  SCIP_DECL_CONSLOCK(scip_lock) {
    getCB()->setWhere(SCIPWhere::ConstraintHandler, "CONSLOCK");
    getCB()->currentCapabilities_ = 0;
    getCB()->run();
    return SCIP_OKAY;
  }

  /** constraint enforcing method of constraint handler for LP solutions
 *
 *  @see SCIP_DECL_CONSENFOLP(x) in @ref type_cons.h
 */
  SCIP_DECL_CONSENFOLP(scip_enfolp) {
    getCB()->actions_ = 0;
    getCB()->setWhere(SCIPWhere::ConstraintHandler, "CONSENFOLP");
    getCB()->currentCapabilities_ = CanDo::ADD_LAZY_CONSTRAINT | CanDo::GET_LP_SOLUTION | CanDo::GET_MIP_SOLUTION;
    sol_ = nullptr;
    getCB()->addPendingConstraints();
    getCB()->run();
    if(getCB()->getActions()==1) *result = SCIP_CONSADDED;
    if (getCB()->getActions() == 2) *result = SCIP_INFEASIBLE;
    return SCIP_OKAY;
  }

  SCIP_DECL_CONSTRANS(scip_trans) {
    getCB()->setWhere(SCIPWhere::ConstraintHandler, "CONSTRANS");
    getCB()->currentCapabilities_ = 0;
    getCB()->run();
    return SCIP_OKAY;
  }

  virtual SCIP_DECL_CONSEXITSOL(scip_exitsol) {
    getCB()->currentCapabilities_ = 0;
    return SCIP_OKAY;
  }
};



} // namespace
#endif // SCIP_CALLBACK_H_INCLUDE_
