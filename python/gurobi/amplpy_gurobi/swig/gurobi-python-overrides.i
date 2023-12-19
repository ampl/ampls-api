%pythoncode %{

# The functions are defined in common-python-overrides.i
# Call update_common.sh to update them in all solvers

GurobiModel._getStatus=GurobiModel.getStatus
GurobiModel.getStatus=lambda self : Status(self._getStatus())
GurobiModel.get_status=lambda self : Status(self._getStatus())

GurobiModel._setAMPLParameter=GurobiModel.setAMPLParameter
GurobiModel.set_ampl_parameter=__setAMPLParameter
GurobiModel.setAMPLParameter=__setAMPLParameter

GurobiModel.getAMPLParameter=__get_ampl_parameter
GurobiModel.get_ampl_parameter=__get_ampl_parameter

GurobiModel.get_ampl_attribute=__get_ampl_attribute
GurobiModel.getAMPLAttribute=__get_ampl_attribute

GurobiCallback._canDo=GurobiCallback.canDo
GurobiCallback.canDo=_do_can_do
GurobiCallback.can_do=_do_can_do

GurobiCallback._getAMPLWhere=GurobiCallback.getAMPLWhere
GurobiCallback.get_ampl_where=lambda self : Where(self._getAMPLWhere())
GurobiCallback.getAMPLWhere=lambda self : Where(self._getAMPLWhere())
%}
