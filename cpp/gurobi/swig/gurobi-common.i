
%{
  #include "gurobi_c.h"
  #include "gurobi_callback.h"
  #include "gurobi_interface.h"
%}

%feature("director") ampls::GurobiCallback;
%ignore ampls::GurobiCallback::doAddCut;
%ignore GRBloadenv;
%ignore AMPL_GRBloadenv2;


// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::GurobiCallback::getSolution;
%template(SolverDriverGrb) ampls::impl::SolverDriver<ampls::GurobiModel>;

%include "gurobi_c.h"
%include "gurobi_callback.h"
%include "gurobi_interface.h"
