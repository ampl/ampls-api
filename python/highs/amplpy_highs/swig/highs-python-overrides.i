%pythoncode %{

# The functions are defined in common-python-overrides.i
# Call update_common.sh to update them in all solvers

HighsModel._getStatus=HighsModel.getStatus
HighsModel.getStatus=lambda self : Status(self._getStatus())
HighsModel.get_status=lambda self : Status(self._getStatus())

HighsModel._setAMPLParameter=HighsModel.setAMPLParameter
HighsModel.set_ampl_parameter=__setAMPLParameter
HighsModel.setAMPLParameter=__setAMPLParameter

HighsModel.getAMPLParameter=__get_ampl_parameter
HighsModel.get_ampl_parameter=__get_ampl_parameter

HighsModel.get_ampl_attribute=__get_ampl_attribute
HighsModel.getAMPLAttribute=__get_ampl_attribute

HighsCallback._canDo=HighsCallback.canDo
HighsCallback.canDo=_do_can_do
HighsCallback.can_do=_do_can_do

HighsCallback._getAMPLWhere=HighsCallback.getAMPLWhere
HighsCallback.get_ampl_where=lambda self : Where(self._getAMPLWhere())
HighsCallback.getAMPLWhere=lambda self : Where(self._getAMPLWhere())
%}
