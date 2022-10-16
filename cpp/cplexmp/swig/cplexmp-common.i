%{
  #include "ilcplex/cplexx.h"
  #include "cplexmp_callback.h"
  #include "cplexmp_interface.h"
%}

%feature("director") ampls::CPLEXCallback;

%ignore ampls::CPLEXCallback::doAddCut;
%ignore ampls::CPLEXDrv::loadModelImpl;
%ignore ampls::cpx::impl;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%template(SolverDriverCPLEX) ampls::impl::SolverDriver<ampls::CPLEXModel>;
%ignore ampls::CPLEXCallback::getSolution;
#define CPXSIZE_BITS 64


%include "ilcplex/cpxconst.h"
%include "ilcplex/cplex.h"
%include "cplexmp_interface.h"
%include "cplexmp_callback.h"