%module(directors="1") amplpy_scip_swig

%include "../../../common-python.i" 
%include "../../../../cpp/ampls/swig/ampls-common.i"


%extend ampls::SCIPModel {
  //TODO
 }

%include "../../../../cpp/scip/swig/scip-common.i"
%include "../../../common-python-extensions.i"
%include "../../../common-python-overrides.i"

%include "scip-python-overrides.i"