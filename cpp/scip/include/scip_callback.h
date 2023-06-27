#ifndef SCIP_CALLBACK_H_INCLUDE_
#define SCIP_CALLBACK_H_INCLUDE_

#include <string>
#include <vector>
#include <map>

#include "ampls/ampls.h"

#include "scip/scip.h"
#include "objscip/objscip.h"

namespace ampls {
namespace impl { namespace scip { class CBWrap; } }
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
class SCIPPresol : public impl::BaseCallback, scip::ObjPresol {
  friend class SCIPModel;
  void* model_;

public:

  SCIPPresol(      
    SCIP*              scip,               /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
    int                maxrounds,          /**< maximal number of presolving rounds the presolver participates in (-1: no limit) */
    SCIP_PRESOLTIMING  timing              /**< timing mask of the presolver */) 
    : scip::ObjPresol(scip, name, desc, priority, maxrounds, timing) {}

  // ************** SCIP specific **************
  /** Get model, useful for calling SCIP c library functions */

  /** Get the underlying SCIP model pointer */
  SCIP* getSCIPModel();
};


class SCIPBranchrule : public impl::BaseCallback, scip::ObjBranchrule {
  friend class SCIPModel;
  void* model_;

public:

  SCIPBranchrule(      
    SCIP*              scip,               /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
    int                maxdepth,           /**< maximal depth level, up to which this branching rule should be used (or -1) */
    double             maxbounddist        /**< maximal relative distance from current node's dual bound to primal bound
                                              *   compared to best node's dual bound for applying branching rule
                                              *   (0.0: only on current best node, 1.0: on all nodes) */) 
    : scip::ObjBranchrule(scip, name, desc, priority, maxdepth, maxbounddist) {}
};


class SCIPSepa : public impl::BaseCallback, scip::ObjSepa {
  friend class SCIPModel;
  void* model_;

public:

  SCIPSepa(      
    SCIP*              scip,               /**< SCIP data structure */
    const char*        name,               /**< name of presolver */
    const char*        desc,               /**< description of presolver */
    int                priority,           /**< priority of the presolver */
      int                freq,               /**< frequency for calling separator */
      SCIP_Real          maxbounddist,       /**< maximal relative distance from current node's dual bound to primal bound compared
                                              *   to best node's dual bound for applying separation */
      SCIP_Bool          usessubscip,        /**< does the separator use a secondary SCIP instance? */
      SCIP_Bool          delay               /**< should separator be delayed, if other separators found cuts? */) 
    : scip::ObjSepa(scip, name, desc, priority, freq, maxbounddist, usessubscip, delay) {}

  // ************** SCIP specific **************
  /** Get model, useful for calling SCIP c library functions */

  /** Get the underlying SCIP model pointer */
  SCIP* getSCIPModel();
};

} // namespace
#endif // SCIP_CALLBACK_H_INCLUDE_
