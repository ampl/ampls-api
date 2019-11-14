
%include "std_string.i"
%include "std_map.i"
%include "std_vector.i"

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

%feature("director") GRBCallback;
%feature("director") GenericCallback;


%ignore BaseCallback::doAddCut;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore GRBCallback::getSolution;
%ignore GenericCallback::getSolution;


%include "gurobi_c.h"
%include "simpleapi/simpleApi.h"
%include "gurobi_callback.h"
%include "gurobi_interface.h"
