%module(directors="1") amplpy_gurobi_swig

%include "../../../common-python.i" 
%include "../../../../cpp/ampls/swig/ampls-common.i"

%extend ampls::GurobiModel {

 int get_int_attr(const char* name) {
      return $self->getIntAttr(name); }
 double get_double_attr(const char* name) {
      return $self->getDoubleAttr(name); }

 int get_int_param(const char* name) {
     return $self->getIntParam(name); }

 double get_double_param(const char* name) {
     return $self->getDoubleParam(name); }

 const char* get_string_param(const char* name) {
     return $self->getStringParam(name); }

 void set_param(const char* name, int value) {
     $self->setParam(name, value); }

 void set_param(const char* name, double value) {
     $self->setParam(name, value); }

 void set_param(const char* name, const char* value) {
     $self->setParam(name, value); } 

 GRBmodel* get_grb_model() { return $self->getGRBmodel(); }

 GRBenv* get_grb_env() {return $self->getGRBenv(); }
 }

%include "../../../../cpp/gurobi/swig/gurobi-common.i"
%include "../../../common-python-extensions.i"



%include "gurobi-python-overrides.i"