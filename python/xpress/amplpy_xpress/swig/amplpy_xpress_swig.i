#ifdef USE_PYTHON_THREADS
%module(directors="1", threads="1") amplpy_xpress_swig
#else
%module(directors="1") amplpy_xpress_swig
#endif


#ifdef USE_PYTHON_THREADS
// Disable threading globally by default
%nothread;
// Selectively enable threading ONLY for methods that need to release GIL
%thread ampls::AMPLModel::optimize;
%thread ampls::impl::BaseCallback::run;
%thread ampls::XPRESSCallback::run;
%thread ampls::GenericCallback::run;
#endif



%include "../../../common-python.i" 
%include "../../../../cpp/ampls/swig/ampls-common.i"

%extend ampls::XPRESSModel {

  int get_int_attr(int attr) {
      return $self->getIntAttr(attr); }

 double get_double_attr(int attr) {
      return $self->getDoubleAttr(attr); }

 int get_int_param(int param) {
     return $self->getIntParam(param); }

 double get_double_param(int param) {
     return $self->getDoubleParam(param); }

 void set_param(int param, int value) {
     $self->setParam(param, value); }

 void set_param(int param, double value) {
     $self->setParam(param, value); }

 XPRSprob get_xprs_prob() { return $self->getXPRSprob(); }
}


%include "../../../../cpp/xpress/swig/xpress-common.i"
%include "../../../common-python-extensions.i"
%include "../../../common-python-overrides.i"

%include "xpress-python-overrides.i"