%pythoncode %{

# The functions are defined in common-python-overrides.i
# Call update_common.sh to update them in all solvers

CPLEXModel._getStatus=CPLEXModel.getStatus
CPLEXModel.getStatus=lambda self : Status(self._getStatus())
CPLEXModel.get_status=lambda self : Status(self._getStatus())

CPLEXModel._setAMPLParameter=CPLEXModel.setAMPLParameter
CPLEXModel.set_ampl_parameter=__setAMPLParameter
CPLEXModel.setAMPLParameter=__setAMPLParameter

CPLEXModel.getAMPLParameter=__get_ampl_parameter
CPLEXModel.get_ampl_parameter=__get_ampl_parameter

CPLEXModel.get_ampl_attribute=__get_ampl_attribute
CPLEXModel.getAMPLAttribute=__get_ampl_attribute

CPLEXCallback._canDo=CPLEXCallback.canDo
CPLEXCallback.canDo=_do_can_do
CPLEXCallback.can_do=_do_can_do

CPLEXCallback._getAMPLWhere=CPLEXCallback.getAMPLWhere
CPLEXCallback.get_ampl_where=lambda self : Where(self._getAMPLWhere())
CPLEXCallback.getAMPLWhere=lambda self : Where(self._getAMPLWhere())
%}
