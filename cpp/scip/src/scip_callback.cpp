#include "scip_callback.h"
#include "scip_interface.h"


namespace ampls
{
  void SCIPCallback::registerPlugins(SCIP* scip) {
    scip_ = scip;
    ch_.reset(new SCIPCH(scip, this));
    auto i = SCIPincludeObjConshdlr(scip, ch_.get(), TRUE);
    /*
    if (dynamic_cast<SCIPHeur*>(callback))
      return SCIPincludeObjHeur(scip, dynamic_cast<SCIPHeur*>(callback), TRUE);
    if (dynamic_cast<SCIPPresol*>(callback))
      return SCIPincludeObjPresol(scip, dynamic_cast<SCIPPresol*>(callback), TRUE);
    if (dynamic_cast<SCIPBranchrule*>(callback))
      return SCIPincludeObjBranchrule(scip, dynamic_cast<SCIPBranchrule*>(callback), TRUE);
    if (dynamic_cast<SCIPSepa*>(callback))
      return SCIPincludeObjSepa(scip, dynamic_cast<SCIPSepa*>(callback), TRUE);
      */
  }
  int SCIPCallback::getSolution(int len, double* sol) {
    if (where_ == ampls::ConstraintHandler)
      return ch_->getSolution(len, sol);
    else
      throw AMPLSolverException("Not implemented in SCIP!");
  }

  int SCIPCallback::doAddCut(const ampls::Constraint& c, int type) {
    actions_ = 1;
    SCIP_VAR** vars = SCIPgetVars(getSCIP());
    SCIP_CONS* cons;
    std::vector<SCIP_VAR*> svars(c.indices().size());
    for (int i = 0; i < c.indices().size(); i++)
      svars[i] = vars[c.indices()[i]];

    double lhss= -SCIP_DEFAULT_INFINITY, rhss= SCIP_DEFAULT_INFINITY;
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
    SCIPcreateConsLinear(getSCIP(), &cons,
      "myname", c.indices().size(), svars.data(), c.coeffs().data(),
      lhss, rhss,0, 0, 1, 1, 1, 0, 0, 0, 1, 0);
    auto r = SCIPaddCons(getSCIP(), cons);
    return 0;
  }

  ampls::Variant SCIPCallback::getValue(ampls::Value::CBValue v) {
    ampls::Variant vv;
    int i = 0;
    switch (v) {
    case Value::N_ROWS:
      return ampls::Variant((int)SCIPgetNConss(getSCIP()));
    default:
      throw std::exception("Not implemented yet");

    }
  }

  double SCIPCallback::getObj() {
    return model_->getObj();
  }

} // namespace

