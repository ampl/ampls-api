#ifndef SCIP_CALLBACK_H_INCLUDE_
#define SCIP_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "scip/scip.h"
#include "objscip/objscip.h"

namespace ampls {

class SCIPPlugin : public impl::BaseCallback {
  SCIP* scip_;
  private:
    int doAddCut(const ampls::Constraint& c, int type) {throw AMPLSolverException("Not implemented in SCIP!");}
    int run() {throw AMPLSolverException("Not implemented in SCIP!");}
    int setHeuristicSolution(int nvars, const int* indices, const double* values) {throw AMPLSolverException("Not implemented in SCIP!");}
    int getSolution(int len, double* sol) {throw AMPLSolverException("Not implemented in SCIP!");}
    double getObj() {throw AMPLSolverException("Not implemented in SCIP!");}
    const char* getWhereString() {throw AMPLSolverException("Not implemented in SCIP!");}
    const char* getMessage() {throw AMPLSolverException("Not implemented in SCIP!");}
    ampls::Where::CBWhere getAMPLWhere() {throw AMPLSolverException("Not implemented in SCIP!");}
    ampls::Variant getValue(ampls::Value::CBValue v) {throw AMPLSolverException("Not implemented in SCIP!");}
    std::vector<double> getValueArray(ampls::Value::CBValue v) {throw AMPLSolverException("Not implemented in SCIP!");}
  public:
    SCIPPlugin(SCIP* scip) {this->scip_ = scip;}
    SCIP* getSCIP();
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
    SCIP_PRESOLTIMING  timing              /**< timing mask of the presolver */) 
    : SCIPPlugin(model), scip::ObjPresol(model, name, desc, priority, maxrounds, timing) {}

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
      SCIP_Bool          usessubscip         /**< does the heuristic use a secondary SCIP instance? */)
      : SCIPPlugin(model), scip::ObjHeur(model, name, desc, dispchar, priority, freq, freqofs, maxdepth, timingmask, usessubscip) {}
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
    double             maxbounddist        /**< maximal relative distance from current node's dual bound to primal bound
                                              *   compared to best node's dual bound for applying branching rule
                                              *   (0.0: only on current best node, 1.0: on all nodes) */) 
    : SCIPPlugin(model), scip::ObjBranchrule(model, name, desc, priority, maxdepth, maxbounddist) {}
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

class SCIPSepa : public SCIPPlugin, scip::ObjSepa {
  friend class SCIPModel;
public:
  SCIPSepa(      
    SCIP*              model,              /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
    int                freq,               /**< frequency for calling separator */
    SCIP_Real          maxbounddist,       /**< maximal relative distance from current node's dual bound to primal bound compared
                                            *   to best node's dual bound for applying separation */
    SCIP_Bool          usessubscip,        /**< does the separator use a secondary SCIP instance? */
    SCIP_Bool          delay               /**< should separator be delayed, if other separators found cuts? */) 
    : SCIPPlugin(model), scip::ObjSepa(model, name, desc, priority, freq, maxbounddist, usessubscip, delay) {}
  private:
    SCIP_DECL_SEPAFREE(scip_free) { return sepa_free(); }
    SCIP_DECL_SEPAINIT(scip_init) { return sepa_init(); }
    SCIP_DECL_SEPAEXIT(scip_exit) { return sepa_exit(); }
    SCIP_DECL_SEPAINITSOL(scip_initsol) { return sepa_initsol(); }
    SCIP_DECL_SEPAEXITSOL(scip_exitsol) { return sepa_exitsol(); }
    SCIP_DECL_SEPAEXECLP(scip_execlp) { return sepa_execlp(result, allowlocal, depth); }
    SCIP_DECL_SEPAEXECSOL(scip_execsol) { return sepa_execsol(sol, result, allowlocal, depth); }

  public:
    virtual SCIP_RETCODE sepa_free() { return SCIP_OKAY; }
    virtual SCIP_RETCODE sepa_init() { return SCIP_OKAY; }
    virtual SCIP_RETCODE sepa_exit() { return SCIP_OKAY; }
    virtual SCIP_RETCODE sepa_initsol() { return SCIP_OKAY; }
    virtual SCIP_RETCODE sepa_exitsol() { return SCIP_OKAY; }
    virtual SCIP_RETCODE sepa_execlp(SCIP_RESULT* result, SCIP_Bool allowlocal, int depth) {
      assert(result != NULL);
      *result = SCIP_DIDNOTRUN;
      return SCIP_OKAY;
    }
    virtual SCIP_RETCODE sepa_execsol(SCIP_SOL* sol, SCIP_RESULT* result, SCIP_Bool allowlocal, int depth) {
      assert(result != NULL);
      *result = SCIP_DIDNOTRUN;
      return SCIP_OKAY;
    }
};

} // namespace
#endif // SCIP_CALLBACK_H_INCLUDE_
