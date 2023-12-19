%pythoncode %{

# The functions are defined in common-python-overrides.i
# Call update_common.sh to update them in all solvers

XPRESSModel._getStatus=XPRESSModel.getStatus
XPRESSModel.getStatus=lambda self : Status(self._getStatus())
XPRESSModel.get_status=lambda self : Status(self._getStatus())

XPRESSModel._setAMPLParameter=XPRESSModel.setAMPLParameter
XPRESSModel.set_ampl_parameter=__setAMPLParameter
XPRESSModel.setAMPLParameter=__setAMPLParameter

XPRESSModel.getAMPLParameter=__get_ampl_parameter
XPRESSModel.get_ampl_parameter=__get_ampl_parameter

XPRESSModel.get_ampl_attribute=__get_ampl_attribute
XPRESSModel.getAMPLAttribute=__get_ampl_attribute

XPRESSCallback._canDo=XPRESSCallback.canDo
XPRESSCallback.canDo=_do_can_do
XPRESSCallback.can_do=_do_can_do

XPRESSCallback._getAMPLWhere=XPRESSCallback.getAMPLWhere
XPRESSCallback.get_ampl_where=lambda self : Where(self._getAMPLWhere())
XPRESSCallback.getAMPLWhere=lambda self : Where(self._getAMPLWhere())
%}
