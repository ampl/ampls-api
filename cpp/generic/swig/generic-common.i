
%include "std_string.i"
%include "std_map.i"
%include "std_vector.i"


%{
  #include "simpleapi/simpleApi.h"
%}

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

%feature("director") ampls::GenericCallback;
%ignore ampls::BaseCallback::doAddCut;
%ignore ampls::GenericCallback::doAddCut;
%ignore ampls::GenericCallback::getSolution;

%include "simpleapi/simpleApi.h"