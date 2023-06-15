%pythoncode %{

# The functions are defined in common-python-extensions.i
# Call update_common.sh to update them in all solvers

CoptModel._getStatus=CoptModel.getStatus
CoptModel.getStatus=lambda self : Status(self._getStatus())
CoptModel.get_status=lambda self : Status(self._getStatus())

CoptModel._setAMPLParameter=CoptModel.setAMPLParameter
CoptModel.set_ampl_parameter=__setAMPLParameter
CoptModel.setAMPLParameter=__setAMPLParameter

CoptModel.getAMPLParameter=__get_ampl_parameter
CoptModel.get_ampl_parameter=__get_ampl_parameter

CoptModel.get_ampl_attribute=__get_ampl_attribute
CoptModel.getAMPLAttribute=__get_ampl_attribute

%}
