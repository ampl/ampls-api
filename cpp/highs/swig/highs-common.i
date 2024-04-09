
%{
  #include "interfaces/highs_c_api.h"
  #include "highs_callback.h"
  #include "highs_interface.h"
%}

%feature("director") ampls::HighsCallback;
%ignore ampls::HighsCallback::doAddCut;
%ignore ampls::impl::highs;
%ignore highs;
// Waiting for a fix for HIGHS_INT64 definition
%ignore HIGHS_WriteBlob;
%ignore HIGHS_ReadBlob;
%ignore HIGHS_SetCallback;
%ignore HIGHS_SetLogCallback;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::HighsCallback::getSolution;
%template(SolverDriverHighs) ampls::impl::SolverDriver<ampls::HighsModel>;

%ignore HIGHS_CloseEnv;
%ignore HIGHS_SetPSDColNames;

%include "interfaces/highs_c_api.h"
%include "highs_callback.h"
%include "highs_interface.h"
