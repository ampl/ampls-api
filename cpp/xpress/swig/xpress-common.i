
%{
  #include "xprs.h"
  #include "xpress_callback.h"
  #include "xpress_interface.h"
%}
// Ignore 64 bits functions as they are confusing SWIG on *nix
%ignore XPRSsetintcontrol64;
%ignore XPRSgetintcontrol64;
%ignore XPRSgetintattrib64;
%ignore XPRSloadlp64;
%ignore XPRSloadqp64;
%ignore XPRSloadqglobal64;
%ignore XPRSloadglobal64;
%ignore XPRSaddpwlcons64;
%ignore XPRSgetpwlcons64;
%ignore XPRSaddgencons64;
%ignore XPRSgetgencons64;
%ignore XPRSgetcols64;
%ignore XPRSgetrows64;
%ignore XPRSgetmqobj64;
%ignore XPRSgetglobal64;
%ignore XPRSaddrows64;
%ignore XPRSaddsets64;
%ignore XPRSaddcols64;
%ignore XPRSaddcuts64;
%ignore XPRSgetcpcuts64;
%ignore XPRSstorecuts64;
%ignore XPRSchgmcoef64;
%ignore XPRSchgmqobj64;
%ignore XPRSaddqmatrix64;
%ignore XPRSloadqcqp64;
%ignore XPRSloadqcqpglobal64;
%ignore XPRSloadmiqcqp64;
%ignore XPRSgetobjintattrib64;
%ignore XPRSloadmiqp64;
%ignore XPRSloadmip64;
%ignore XPRSgetmipentities64;

%feature("director") ampls::XPRESSCallback;
%ignore ampls::impl::xpress;
%ignore ampls::XPRESSCallback::doAddCut;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::XPRESSCallback::getSolution;
%template(SolverDriverXpress) ampls::impl::SolverDriver<ampls::XPRESSModel>;


%include "xprs.h"
%include "xpress_callback.h"
%include "xpress_interface.h"
