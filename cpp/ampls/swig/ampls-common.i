%include "std_string.i"
%include "std_map.i"
%include "std_vector.i"

%{
  #include "ampls/ampls.h"
%}

namespace std {
  %template(map_string_int)map<string, int>;
  %template(map_int_string)map<int, string>;
  %template(map_string_double)map<string, double>;
  %template(vector_string)vector<string>;
  %template(vector_double)vector<double>;
  %template(vector_int)vector<int>;
  %template(vector_options)vector<ampls::Option>;
}

%feature("director") ampls::impl::BaseCallback; 
%feature("director") ampls::GenericCallback;
%ignore ampls::GenericCallback::getSolution;
%ignore ampls::impl::mp;

%include "ampls/ampls.h"