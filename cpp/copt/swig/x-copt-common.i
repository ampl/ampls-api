
%{
  #include "copt_c.h"
  #include "copt_callback.h"
  #include "copt_interface.h"
%}

%feature("director") ampls::CoptCallback;
%ignore ampls::CoptCallback::doAddCut;
%ignore ampls::xgrb::impl;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::CoptCallback::getSolution;
%template(SolverDriverXGrb) ampls::impl::SolverDriver<ampls::CoptModel>;


%include "copt_c.h"
%include "copt_callback.h"
%include "copt_interface.h"
