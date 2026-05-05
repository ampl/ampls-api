#ifdef USE_PYTHON_THREADS
%module(directors="1", threads="1") amplpy_copt_swig
#else
%module(directors="1") amplpy_copt_swig
#endif


#ifdef USE_PYTHON_THREADS
// Disable threading globally by default
%nothread;
// Selectively enable threading ONLY for methods that need to release GIL
%thread ampls::AMPLModel::optimize;
%thread ampls::impl::BaseCallback::run;
%thread ampls::CoptCallback::run;
%thread ampls::GenericCallback::run;
#endif

%include "../../../common-python.i"
%include "../../../../cpp/ampls/swig/ampls-common.i"
%include "../../../../cpp/copt/swig/copt-common.i"

%include "../../../common-python-extensions.i"
%include "../../../common-python-overrides.i"

%include "copt-python-overrides.i"



