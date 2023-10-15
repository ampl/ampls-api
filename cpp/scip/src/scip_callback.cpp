#include "scip_callback.h"
#include "scip_interface.h"

#include <algorithm> // for std::find

#include <assert.h>


/**
Heuristics
*/
struct SCIP_HeurData
{
  ampls::SCIPCallback* cb_;
};

namespace ampls {
  namespace impl {
    namespace scip {
      namespace heur {
        #define HEUR_NAME             "xyz"
        #define HEUR_DESC             "primal heuristic template"
        #define HEUR_DISPCHAR         '?'
        #define HEUR_PRIORITY         100
        #define HEUR_FREQ             1
        #define HEUR_FREQOFS          1
        #define HEUR_MAXDEPTH         -1
        #define HEUR_TIMING           SCIP_HEURTIMING_AFTERNODE
        #define HEUR_USESSUBSCIP      FALSE  /**< does the heuristic use a secondary SCIP instance? */

        #define heurCopyXyz NULL
        #define heurInitXyz NULL
        #define heurExitXyz NULL
        #define heurInitsolXyz NULL
        #define heurExitsolXyz NULL

        /** destructor of primal heuristic to free user data (called when SCIP is exiting) */
        static SCIP_DECL_HEURFREE(heurFreeXyz)
        {
          SCIP_HeurData* heurdata;
          /* free heuristic rule data */
          heurdata = SCIPheurGetData(heur);
          SCIPfreeBlockMemory(scip, &heurdata);
          SCIPheurSetData(heur, NULL);
          return SCIP_OKAY;
        }
        
          static
            SCIP_DECL_HEUREXEC(heurExecXyz)
          {
            auto heurdata = SCIPheurGetData(heur);
            auto cb = heurdata->cb_;
            cb->setWhere(ampls::SCIPWhere::Heuristics, "HEUREXEC");
            cb->currentCapabilities_ = CanDo::IMPORT_SOLUTION | CanDo::GET_LP_SOLUTION;
            cb->run();

            if (cb->actions_ == 4)
            {
              SCIP_SOL* sol;
              SCIP_VAR* tvar;
              SCIPcreateSol(cb->getSCIP(), &sol, heur);
              for (int i = 0; i < cb->solutionToSet_.indices.size(); i++)
              {
                SCIPgetTransformedVar(cb->getSCIP(), cb->modelToScip()[cb->solutionToSet_.indices[i]], &tvar);
                SCIPsetSolVal(cb->getSCIP(), sol, tvar, cb->solutionToSet_.vars[i]);
              }
              unsigned int stored;
              SCIPtrySolFree(cb->getSCIP(), &sol, 0, 0, 1, 1, 1, &stored);

              cb->solutionToSet_.vars.clear();
              cb->solutionToSet_.indices.clear();
              cb->actions_ = 0;
              if (stored)
                *result = SCIP_FOUNDSOL;
              else
                *result = SCIP_DIDNOTFIND;
            }
            else
              *result = SCIP_DIDNOTRUN;
            return SCIP_OKAY;
          }

          SCIP_RETCODE SCIPincludeHeurXyz(
            SCIP* scip,
            ampls::SCIPCallback* cb/**< SCIP data structure */
          )
          {
            SCIP_HEURDATA* heurdata = NULL;
            SCIP_HEUR* heur;

            /* create xyz primal heuristic data */
            SCIP_CALL(SCIPallocBlockMemory(scip, &heurdata));
            heurdata->cb_ = cb;
            heur = NULL;

            /* include primal heuristic */
            SCIP_CALL(SCIPincludeHeurBasic(scip, &heur,
              HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, HEUR_FREQ, HEUR_FREQOFS,
              HEUR_MAXDEPTH, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecXyz, heurdata));

            assert(heur != NULL);
            SCIP_CALL(SCIPsetHeurCopy(scip, heur, heurCopyXyz));
            SCIP_CALL(SCIPsetHeurFree(scip, heur, heurFreeXyz));
            SCIP_CALL(SCIPsetHeurInit(scip, heur, heurInitXyz));
            SCIP_CALL(SCIPsetHeurExit(scip, heur, heurExitXyz));
            SCIP_CALL(SCIPsetHeurInitsol(scip, heur, heurInitsolXyz));
            SCIP_CALL(SCIPsetHeurExitsol(scip, heur, heurExitsolXyz));

            return SCIP_OKAY;
          }

      } // heur
    } // scip
  } // impl
} // ampls


namespace ampls
{

    SCIP_VAR** SCIPCallback::modelToScip() {
    return ((SCIPModel*)this->model_)->originalVars();
  }
  void SCIPCallback::registerPlugins(SCIP* scip) {
    scip_ = scip;
    ch_.reset(new SCIPCH(scip, this));
    auto i = SCIPincludeObjConshdlr(scip, ch_.get(), TRUE);
    auto rv = impl::scip::heur::SCIPincludeHeurXyz(scip, this);

  }
  int SCIPCallback::getSolution(int len, double* sol) {
    if (where_ == ampls::ConstraintHandler)
      return ch_->getSolution(len, sol, ((SCIPModel*)model_)->originalVars());
    else
      throw AMPLSolverException("Not implemented in SCIP!");
  }
  void SCIPCallback::addPendingConstraints() {
    for (auto i : pendingConstraints_)
      addConstraintToScip(cachedCons_[i], 1);
    pendingConstraints_.clear();
  }

  int SCIPCallback::doAddCut(const ampls::Constraint& c, int type) {
    if (std::find(cachedCons_.begin(), cachedCons_.end(), c) != cachedCons_.end()) {
        actions_ = 2;
        return 0;
    }
    cachedCons_.push_back(c);
    actions_ = 1;

    if (whereString_ == "CONSCHECK")
      pendingConstraints_.push_back(cachedCons_.size() - 1);
    else
      addConstraintToScip(c, type);

    return 0;
  }

  void SCIPCallback::addConstraintToScip(const ampls::Constraint& c, int type) {
    SCIP_VAR* tvar;
    SCIP_CONS* cons;
    auto size = c.indices().size();
    std::vector< SCIP_VAR*> svars(size);
    for (int i = 0; i < size; i++)
    {
      SCIPgetTransformedVar(getSCIP(), modelToScip()[c.indices()[i]], &tvar);
      svars[i] = tvar;
    }

    double lhss = -SCIP_DEFAULT_INFINITY, rhss = SCIP_DEFAULT_INFINITY;
    switch (c.sense()) {
    case ampls::CutDirection::EQ:
      rhss = lhss = c.rhs();
      break;
    case ampls::CutDirection::GE:
      lhss = c.rhs();
      break;
    case ampls::CutDirection::LE:
      rhss = c.rhs();
      break;
    }

    SCIP_Bool             initial = type != 1;            /**< should the LP relaxation of constraint be in the initial LP?*/
    SCIP_Bool             separate = TRUE;           /**< should the constraint be separated during LP processing? */
    SCIP_Bool             enforce = TRUE;            /**< should the constraint be enforced during node processing? */
    SCIP_Bool             check = TRUE;              /**< should the constraint be checked for feasibility? */
    SCIP_Bool             propagate = TRUE;          /**< should the constraint be propagated during node processing? */
    SCIP_Bool             local = FALSE;             /**< is constraint only valid locally?*/
    SCIP_Bool             modifiable = FALSE;         /**< is constraint modifiable (subject to column generation)?  Usually set to FALSE. In column generation applications, set to TRUE if pricing *   adds coefficients to this constraint. */
    SCIP_Bool             dynamic = FALSE;            /**< is constraint subject to aging?*/
    SCIP_Bool             removable = TRUE;          /**< should the relaxation be removed from the LP due to aging or cleanup? *   Usually set to FALSE. Set to TRUE for 'lazy constraints' and 'user cuts'. */
    SCIP_Bool             stickingatnode = FALSE;
    
    std::string name = type == 1 ? "lazy_" : "cut_" + std::to_string(cachedCons_.size());

    SCIPcreateConsLinear(getSCIP(), &cons,
      name.c_str(), size, svars.data(), c.coeffs().data(),
      lhss, rhss, initial, separate, enforce, check, propagate, local,
      modifiable, dynamic, removable, stickingatnode);

    auto r = SCIPaddCons(getSCIP(), cons);
    SCIPreleaseCons(getSCIP(), &cons);
  }

  ampls::Variant SCIPCallback::getValueImpl(ampls::Value::CBValue v) {
    ampls::Variant vv;
    int i = 0;
    switch (v) {
    case Value::N_ROWS:
      return ampls::Variant((int)SCIPgetNConss(getSCIP()));
    case Value::N_COLS:
      return ampls::Variant((int)SCIPgetNVars(getSCIP()));
    default:
      throw std::exception("Not implemented yet");

    }
  }

  double SCIPCallback::getObj() {
    return model_->getObj();
  }

} // namespace

