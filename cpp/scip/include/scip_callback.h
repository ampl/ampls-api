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
    SCIP*         model,               /**< SCIP data structure */
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
    SCIP*         model,               /**< SCIP data structure */
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
};

class SCIPBranchrule : public SCIPPlugin, scip::ObjBranchrule {
  friend class SCIPModel;
public:
  SCIPBranchrule(      
    SCIP*          model,               /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
    int                maxdepth,           /**< maximal depth level, up to which this branching rule should be used (or -1) */
    double             maxbounddist        /**< maximal relative distance from current node's dual bound to primal bound
                                              *   compared to best node's dual bound for applying branching rule
                                              *   (0.0: only on current best node, 1.0: on all nodes) */) 
    : SCIPPlugin(model), scip::ObjBranchrule(model, name, desc, priority, maxdepth, maxbounddist) {}
};

class SCIPSepa : public SCIPPlugin, scip::ObjSepa {
  friend class SCIPModel;
public:
  SCIPSepa(      
    SCIP*          model,               /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
    int                freq,               /**< frequency for calling separator */
    SCIP_Real          maxbounddist,       /**< maximal relative distance from current node's dual bound to primal bound compared
                                            *   to best node's dual bound for applying separation */
    SCIP_Bool          usessubscip,        /**< does the separator use a secondary SCIP instance? */
    SCIP_Bool          delay               /**< should separator be delayed, if other separators found cuts? */) 
    : SCIPPlugin(model), scip::ObjSepa(model, name, desc, priority, freq, maxbounddist, usessubscip, delay) {}
};

} // namespace
#endif // SCIP_CALLBACK_H_INCLUDE_
