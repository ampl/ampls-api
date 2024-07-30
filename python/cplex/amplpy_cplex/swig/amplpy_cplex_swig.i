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
%include "../../../common-python-overrides.i"

// Aliases for CPLEX specific callback signatures
%pythoncode %{
def do_addCut_cplex(self, vars, coeffs, direction, rhs, local=0):
    return self._addCut(vars, coeffs, __e_to_v(direction), rhs, local)
def do_addLazy_cplex(self, vars, coeffs, direction, rhs,local=0):
    return self._addLazy(vars, coeffs, __e_to_v(direction), rhs, local)
def do_addCutIndices_cplex(self, nvars, coeffs, direction, rhs,local=0):
    return self._addCutIndices(nvars, coeffs, __e_to_v(direction), rhs, local)
def do_addLazyIndices_cplex(self, nvars, coeffs, direction, rhs, local=0):
    return self._addLazyIndices(nvars, coeffs, __e_to_v(direction), rhs, local)


CPLEXCallback._addLazy=CPLEXCallback.addLazy
CPLEXCallback._addCut=CPLEXCallback.addCut
CPLEXCallback._addLazyIndices=CPLEXCallback.addLazyIndices
CPLEXCallback._addCutIndices=CPLEXCallback.addCutIndices
CPLEXCallback.addLazy= do_addLazy_cplex
CPLEXCallback.addCut= do_addCut_cplex
CPLEXCallback.addCutIndices= do_addCutIndices_cplex
CPLEXCallback.addLazyIndices= do_addLazyIndices_cplex
CPLEXCallback.add_lazy= do_addLazy_cplex
CPLEXCallback.add_cut= do_addCut_cplex
CPLEXCallback.add_cut_indices= do_addCutIndices_cplex
CPLEXCallback.add_lazy_indices= do_addLazyIndices_cplex
%}


%include "cplex-python-overrides.i"


