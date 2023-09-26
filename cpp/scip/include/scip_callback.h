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
  class SCIPCH;
  class SCIPModel;

  enum SCIPWhere {
    ConstraintHandler
  };


  class SCIPCallback : public impl::BaseCallback {
    friend class SCIPCH;
    friend class SCIPModel;
    SCIP* scip_;
    std::string whereString_;
    std::shared_ptr<SCIPCH> ch_;
    int actions_;
    void setWhere(SCIPWhere where, const char* str) {
      where_ = where;
      whereString_ = str;
    }
    void registerPlugins(SCIP* scip);
  public:
    int getActions() { return actions_; }
    int doAddCut(const ampls::Constraint& c, int type);
    virtual int run() = 0;
    int setHeuristicSolution(int nvars, const int* indices, const double* values) { throw AMPLSolverException("Not implemented in SCIP!"); }
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
    ampls::Variant getValue(ampls::Value::CBValue v);
    std::vector<double> getValueArray(ampls::Value::CBValue v) { throw AMPLSolverException("Not implemented in SCIP!"); }
  public:
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

class SCIPHeur : public SCIPPlugin, scip::ObjHeur {
  friend class SCIPModel;
  public: 
    SCIPHeur(      
      SCIP*              model,              /**< SCIP data structure */
      const char*        name,               /**< name of presolver */
      const char*        desc,               /**< description of presolver */
      char               dispchar,           /**< display character of primal heuristic */
      int                priority,           /**< priority of the primal heuristic */
      int                freq,               /**< frequency for calling primal heuristic */
      int                freqofs,            /**< frequency offset for calling primal heuristic */
      int                maxdepth,           /**< maximal depth level to call heuristic at (-1: no limit) */
      SCIP_HEURTIMING    timingmask,         /**< positions in the node solving loop where heuristic should be executed;
                                              *   see definition of SCIP_HEURTIMING for possible values */
      SCIP_Bool          usessubscip,   
    /**< does the heuristic use a secondary SCIP instance? */
      SCIPCallback* cb) 
      : SCIPPlugin(model, cb), scip::ObjHeur(model, name, desc, dispchar, priority, freq, freqofs, maxdepth, timingmask, usessubscip) {}
  private:
    SCIP_DECL_HEURFREE(scip_free) { return heur_free(); }
    SCIP_DECL_HEURINIT(scip_init) { return heur_init(); }
    SCIP_DECL_HEUREXIT(scip_exit) { return heur_exit(); }
    SCIP_DECL_HEURINITSOL(scip_initsol) { return heur_initsol(); }
    SCIP_DECL_HEUREXITSOL(scip_exitsol) { return heur_exitsol(); }
    SCIP_DECL_HEUREXEC(scip_exec) { return heur_exec(heurtiming, nodeinfeasible, result); }

  public:
    virtual SCIP_RETCODE heur_free() { return SCIP_OKAY; }
    virtual SCIP_RETCODE heur_init() { return SCIP_OKAY; }
    virtual SCIP_RETCODE heur_exit() { return SCIP_OKAY; }
    virtual SCIP_RETCODE heur_initsol() { return SCIP_OKAY; }
    virtual SCIP_RETCODE heur_exitsol() { return SCIP_OKAY; }
    virtual SCIP_RETCODE heur_exec(SCIP_HEURTIMING heurtiming,
      SCIP_Bool nodeinfeasible, SCIP_RESULT* result) = 0;
};

class SCIPBranchrule : public SCIPPlugin, scip::ObjBranchrule {
  friend class SCIPModel;
public:
  SCIPBranchrule(      
    SCIP*              model,              /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
    int                maxdepth,           /**< maximal depth level, up to which this branching rule should be used (or -1) */
    double             maxbounddist,        /**< maximal relative distance from current node's dual bound to primal bound
                                              *   compared to best node's dual bound for applying branching rule
                                              *   (0.0: only on current best node, 1.0: on all nodes) */
      SCIPCallback* cb)
    : SCIPPlugin(model, cb), scip::ObjBranchrule(model, name, desc, priority, maxdepth, maxbounddist) {}
  private:
    SCIP_DECL_BRANCHFREE(scip_free) { return branch_free(); }
    SCIP_DECL_BRANCHINIT(scip_init) { return branch_init(); }
    SCIP_DECL_BRANCHEXIT(scip_exit) { return branch_exit(); }
    SCIP_DECL_BRANCHINITSOL(scip_initsol) { return branch_initsol(); }
    SCIP_DECL_BRANCHEXITSOL(scip_exitsol) { return branch_exitsol(); }
    SCIP_DECL_BRANCHEXECLP(scip_execlp) { return branch_execlp(allowaddcons, result); }
    SCIP_DECL_BRANCHEXECEXT(scip_execext) { return branch_execext(allowaddcons, result); }
    SCIP_DECL_BRANCHEXECPS(scip_execps) { return branch_execps(allowaddcons, result); }

  public:
    virtual SCIP_RETCODE branch_free() { return SCIP_OKAY; }
    virtual SCIP_RETCODE branch_init() { return SCIP_OKAY; }
    virtual SCIP_RETCODE branch_exit() { return SCIP_OKAY; }
    virtual SCIP_RETCODE branch_initsol() { return SCIP_OKAY; }
    virtual SCIP_RETCODE branch_exitsol() { return SCIP_OKAY; }
    virtual SCIP_RETCODE branch_execlp(SCIP_Bool allowaddcons, SCIP_RESULT* result) {
      assert(result != NULL);
      *result = SCIP_DIDNOTRUN;
      return SCIP_OKAY;
    }
    virtual SCIP_RETCODE branch_execext(SCIP_Bool allowaddcons, SCIP_RESULT* result) {
      assert(result != NULL);
      *result = SCIP_DIDNOTRUN;
      return SCIP_OKAY;
    }
    virtual SCIP_RETCODE branch_execps(SCIP_Bool allowaddcons, SCIP_RESULT* result) {
      assert(result != NULL);
      *result = SCIP_DIDNOTRUN;
      return SCIP_OKAY;
    }
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
    int getSolution(int len, double* sol) {
      SCIP_VAR** vars = SCIPgetVars(getSCIP());
      for (auto i = 0; i < len; ++i)
        sol[i] = SCIPgetSolVal(getSCIP(), sol_, vars[i]);
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
    getCB()->currentCapabilities_ = CanDo::GET_LP_SOLUTION | CanDo::GET_MIP_SOLUTION;
    sol_ = sol;
    getCB()->run();
    *result= SCIP_INFEASIBLE;
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
    getCB()->run();
    if(getCB()->getActions()==1)
    *result = SCIP_CONSADDED;
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
//
//class SCIPSepa : public SCIPPlugin, scip::ObjSepa {
//  
//  friend class SCIPModel;
//public:
//  SCIPSepa(      
//    SCIP*              model,              /**< SCIP data structure */
//    const char*        name,               /**< name of presolver */
//    const char*        desc,               /**< description of presolver */
//    int                priority,           /**< priority of the presolver */
//    int                freq,               /**< frequency for calling separator */
//    SCIP_Real          maxbounddist,       /**< maximal relative distance from current node's dual bound to primal bound compared
//                                            *   to best node's dual bound for applying separation */
//    SCIP_Bool          usessubscip,        /**< does the separator use a secondary SCIP instance? */
//    SCIP_Bool          delay               /**< should separator be delayed, if other separators found cuts? */) 
//    : SCIPPlugin(model), scip::ObjSepa(model, name, desc, priority, freq, maxbounddist, usessubscip, delay) {}
//  private:
//    SCIP_DECL_SEPAFREE(scip_free) { return sepa_free(); }
//    SCIP_DECL_SEPAINIT(scip_init) { return sepa_init(); }
//    SCIP_DECL_SEPAEXIT(scip_exit) { return sepa_exit(); }
//    SCIP_DECL_SEPAINITSOL(scip_initsol) { return sepa_initsol(); }
//    SCIP_DECL_SEPAEXITSOL(scip_exitsol) { return sepa_exitsol(); }
//    SCIP_DECL_SEPAEXECLP(scip_execlp) { return sepa_execlp(result, allowlocal, depth); }
//    SCIP_DECL_SEPAEXECSOL(scip_execsol) { return sepa_execsol(sol, result, allowlocal, depth); }
//
//  public:
//    virtual SCIP_RETCODE sepa_free() { return SCIP_OKAY; }
//    virtual SCIP_RETCODE sepa_init() { return SCIP_OKAY; }
//    virtual SCIP_RETCODE sepa_exit() { return SCIP_OKAY; }
//    virtual SCIP_RETCODE sepa_initsol() { return SCIP_OKAY; }
//    virtual SCIP_RETCODE sepa_exitsol() { return SCIP_OKAY; }
//    virtual SCIP_RETCODE sepa_execlp(SCIP_RESULT* result, SCIP_Bool allowlocal, int depth) {
//      assert(result != NULL);
//      *result = SCIP_DIDNOTRUN;
//      return SCIP_OKAY;
//    }
//    virtual SCIP_RETCODE sepa_execsol(SCIP_SOL* sol, SCIP_RESULT* result, SCIP_Bool allowlocal, int depth) {
//      assert(result != NULL);
//      *result = SCIP_DIDNOTRUN;
//      return SCIP_OKAY;
//    }
//};




} // namespace
#endif // SCIP_CALLBACK_H_INCLUDE_
