%pythoncode %{

# The functions are defined in common-python-overrides.i
# Call update_common.sh to update them in all solvers

SCIPModel._getStatus=SCIPModel.getStatus
SCIPModel.getStatus=lambda self : Status(self._getStatus())
SCIPModel.get_status=lambda self : Status(self._getStatus())

SCIPModel._setAMPLParameter=SCIPModel.setAMPLParameter
SCIPModel.set_ampl_parameter=__setAMPLParameter
SCIPModel.setAMPLParameter=__setAMPLParameter

SCIPModel.getAMPLParameter=__get_ampl_parameter
SCIPModel.get_ampl_parameter=__get_ampl_parameter

SCIPModel.get_ampl_attribute=__get_ampl_attribute
SCIPModel.getAMPLAttribute=__get_ampl_attribute

SCIPCalllback._canDo=SCIPCalllback.canDo
SCIPCalllback.canDo=_do_can_do
SCIPCalllback.can_do=_do_can_do

SCIPCalllback._getAMPLWhere=SCIPCalllback.getAMPLWhere
SCIPCalllback.get_ampl_where=lambda self : Where(self._getAMPLWhere())
SCIPCalllback.getAMPLWhere=lambda self : Where(self._getAMPLWhere())
%}
