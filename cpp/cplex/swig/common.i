
%include "std_string.i"
%include "std_map.i"
%include "std_vector.i"

namespace std {
  %template(map_string_int)map<string, int>;
  %template(vector_string)vector<string>;
}

%{
#include "cplex.h"
#include "cplexx.h"
#include "simpleapi/amplInterface.h"
#include "cplex_callback.h"
#include "cplex_interface.h"
%}

%feature("director") CPLEXCallback;
%feature("director") GenericCallback;

%ignore msg_callback_wrapper;
%ignore lp_callback_wrapper;
%ignore setDefaultCB;
%ignore cut_callback_wrapper;
%ignore incumbent_callback_wrapper;


%ignore BaseCallback::doAddCut;
// The following is to avoid problem with director
// that cannot properly map an out double[].
// Not yet.
%ignore CPLEXCallback::getSolution;
%ignore GenericCallback::getSolution;

//%include "cplex.h"
%include "simpleapi/amplInterface.h"
%include "cplex_interface.h"
%include "cplex_callback.h"
