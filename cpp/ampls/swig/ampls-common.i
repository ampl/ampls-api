
%include "std_string.i"
%include "std_map.i"
%include "std_vector.i"


%{
  #include "ampls/ampls.h"
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
  %template(map_int_string)map<int, string>;
  %template(map_string_double)map<string, double>;
  %template(vector_string)vector<string>;
  %template(vector_double)vector<double>;
}

%feature("director") ampls::GenericCallback;
%ignore ampls::BaseCallback::doAddCut;
%ignore ampls::GenericCallback::doAddCut;
%ignore ampls::GenericCallback::getSolution;
%ignore ampls::impl::mp;
%ignore ampls::impl::mp::AMPLSSetDblOption;
%ignore ampls::impl::mp::AMPLSSetStrOption;
%ignore ampls::impl::mp::AMPLSGetDblOption;
%ignore ampls::impl::mp::AMPLSGetStrOption;

%include "ampls/ampls.h"