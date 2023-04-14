
%{
  #include "Cbc_C_Interface.h"
  #include "cbcmp_callback.h"
  #include "cbcmp_interface.h"
%}

%feature("director") ampls::CbcCallback;
%ignore ampls::CbcCallback::doAddCut;
%ignore ampls::xgrb::impl;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::CbcCallback::getSolution;
%template(SolverDriverXGrb) ampls::impl::SolverDriver<ampls::CbcModel>;


%include "Cbc_C_Interface.h"
%include "cbcmp_callback.h"
%include "cbcmp_interface.h"
