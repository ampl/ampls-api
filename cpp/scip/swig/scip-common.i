%{
  #include "scip/scip.h"
  #include "scip_callback.h"
  #include "scip_interface.h"
%}

%feature("director") ampls::SCIPCallback;
%ignore ampls::SCIPCallback::doAddCut;
%ignore ampls::impl::scip;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::SCIPCallback::getSolution;
%template(SolverDriverSCIP) ampls::impl::SolverDriver<ampls::SCIPModel>;

%include "scip_callback.h"
%include "scip_interface.h"
%include "scip/scip.h"
