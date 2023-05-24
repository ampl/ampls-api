%module(directors="1") amplpy_cplex_swig

%include "../../../common-python.i"
%include "../../../../cpp/ampls/swig/ampls-common.i"

%extend ampls::CPLEXModel {

 int get_int_param(int param) {
     return $self->getIntParam(param); }

 double get_double_param(int param) {
     return $self->getDoubleParam(param); }

 void set_param(int param, int value) {
     $self->setParam(param, value); }

 void set_param(int param, double value) {
     $self->setParam(param, value); }

 CPXLPptr get_cplex_lp() { return $self->getCPXLP(); }

 CPXENVptr get_cplex_env() { return $self->getCPXENV(); }
}


%include "../../../../cpp/cplex/swig/cplex-common.i"
%include "../../../common-python-extensions.i"
