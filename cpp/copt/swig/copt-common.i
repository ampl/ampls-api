
%{
  #include "copt.h"
  #include "copt_callback.h"
  #include "copt_interface.h"
%}

%feature("director") ampls::CoptCallback;
%ignore ampls::CoptCallback::doAddCut;
%ignore ampls::impl::copt;

// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::CoptCallback::getSolution;
%template(SolverDriverCopt) ampls::impl::SolverDriver<ampls::CoptModel>;

%ignore COPT_CloseEnv;
%ignore COPT_SetPSDColNames;

%include "copt.h"
%include "copt_callback.h"
%include "copt_interface.h"