
%{
  #include "xprs.h"
  #include "xpress_callback.h"
  #include "xpress_interface.h"
%}

%feature("director") ampls::XPRESSCallback;
%ignore ampls::XPRESSCallback::doAddCut;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::XPRESSCallback::getSolution;
%template(SolverDriverXpress) ampls::impl::SolverDriver<ampls::XPRESSModel>;


%include "xprs.h"
%include "xpress_callback.h"
%include "xpress_interface.h"
