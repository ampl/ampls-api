
%{
  #include "gurobi_c.h"
  #include "x-gurobi_callback.h"
  #include "x-gurobi_interface.h"
%}

%feature("director") ampls::XGurobiCallback;
%ignore ampls::XGurobiCallback::doAddCut;
%ignore ampls::xgrb::impl;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::XGurobiCallback::getSolution;
%template(SolverDriverXGrb) ampls::impl::SolverDriver<ampls::XGurobiModel>;


%include "gurobi_c.h"
%include "x-gurobi_callback.h"
%include "x-gurobi_interface.h"
