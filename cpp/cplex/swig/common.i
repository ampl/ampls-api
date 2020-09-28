
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
  #include "ilcplex/cplex.h"
  #include "ilcplex/cplexx.h"
  #include "simpleapi/simpleApi.h"
  #include "cplex_callback.h"
  #include "cplex_interface.h"
%}

%feature("director") ampls::CPLEXCallback;
%feature("director") ampls::GenericCallback;

%ignore ampls::msg_callback_wrapper;
%ignore ampls::lp_callback_wrapper;
%ignore ampls::setDefaultCB;
%ignore ampls::cut_callback_wrapper;
%ignore ampls::incumbent_callback_wrapper;

%ignore ampls::BaseCallback::doAddCut;
%ignore ampls::GenericCallback::doAddCut;
%ignore ampls::CPLEXCallback::doAddCut;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore ampls::CPLEXCallback::getSolution;
%ignore ampls::GenericCallback::getSolution;

//%include "cplex.h"
%include "simpleapi/simpleApi.h"
%include "cplex_interface.h"
%include "cplex_callback.h"
