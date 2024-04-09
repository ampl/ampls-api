%module(directors="1") amplpy_highs_swig

%include "../../../common-python.i" 
%include "../../../../cpp/ampls/swig/ampls-common.i"

%extend ampls::HighsModel {

int get_int64_attr(const char* name) {
      return $self->getInt64Attr(name); }
 int get_int_attr(const char* name) {
      return $self->getIntAttr(name); }
 double get_double_attr(const char* name) {
      return $self->getDoubleAttr(name); }

int get_bool_param(const char* name) {
     return $self->getBoolParam(name); }

 int get_int_param(const char* name) {
     return $self->getIntParam(name); }

 double get_double_param(const char* name) {
     return $self->getDoubleParam(name); }

 std::string get_string_param(const char* name) {
     return $self->getStringParam(name); }

 void set_param(const char* name, bool value) {
     $self->setParam(name, value); }

 void set_param(const char* name, int value) {
     $self->setParam(name, value); }

 void set_param(const char* name, double value) {
     $self->setParam(name, value); }

 void set_param(const char* name, const char* value) {
     $self->setParam(name, value); } 

 void* get_highs_model() { return $self->getHighsModel(); }
  }

%include "../../../../cpp/highs/swig/highs-common.i"
%include "../../../common-python-extensions.i"



%include "highs-python-overrides.i"