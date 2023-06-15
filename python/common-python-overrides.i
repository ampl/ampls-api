%pythoncode %{

# The functions are defined in common-python-extensions.i
# Call update_common.sh to update them in all solvers

AMPLModel._getStatus=AMPLModel.getStatus
AMPLModel.getStatus=lambda self : Status(self._getStatus())
AMPLModel.get_status=lambda self : Status(self._getStatus())

AMPLModel._setAMPLParameter=AMPLModel.setAMPLParameter
AMPLModel.set_ampl_parameter=__setAMPLParameter
AMPLModel.setAMPLParameter=__setAMPLParameter

AMPLModel.getAMPLParameter=__get_ampl_parameter
AMPLModel.get_ampl_parameter=__get_ampl_parameter

AMPLModel.get_ampl_attribute=__get_ampl_attribute
AMPLModel.getAMPLAttribute=__get_ampl_attribute

%}
