
%include "std_string.i"
%include "std_map.i"
%include "std_vector.i"

%include exception.i       
%exception {
  try {
    $action
  } catch(const ampls::AMPLSolverException &e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  } 
  catch(...) {
    SWIG_exception(SWIG_RuntimeError, "Unknown exception");
  }
}

namespace std {
  %template(map_string_int)map<string, int>;
  %template(vector_string)vector<string>;
}

%{
  #include "gurobi_c.h"
  #include "simpleapi/simpleApi.h"
  #include "gurobi_callback.h"
  #include "gurobi_interface.h"
%}

%feature("director") ampls::GurobiCallback;
%feature("director") ampls::GenericCallback;

%ignore ampls::BaseCallback::doAddCut;
%ignore ampls::GenericCallback::doAddCut;
%ignore ampls::GurobiCallback::doAddCut;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::GurobiCallback::getSolution;
%ignore ampls::GenericCallback::getSolution;


%include "gurobi_c.h"
%include "simpleapi/simpleApi.h"
%include "gurobi_callback.h"
%include "gurobi_interface.h"
