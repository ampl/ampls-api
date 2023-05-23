

%rename("%s") getVarMap;
%rename("%s") getVarMapInverse;
%rename("%s") getVarMapFiltered;

%extend ampls::AMPLModel{
  PyObject* getSolutionDict() {
    PyObject* res = PyDict_New();
    std::vector<double> sol = self->getSolutionVector();
    std::map<int, std::string> map = self->getVarMapInverse();
    std::map<int, std::string>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
      PyDict_SetItem(res, PyString_FromString(it->second.c_str()), PyFloat_FromDouble(sol[it->first]));
    }
    return res;
  }

  PyObject* getVarMap() {
    PyObject* res = PyDict_New();
    std::map<std::string, int> map = self->getVarMap();
    std::map<std::string, int>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
      PyDict_SetItem(res, PyString_FromString(it->first.c_str()), PyInt_FromLong(it->second));
    }
    return res;
  }
  PyObject* getVarMapFiltered(const char* beginWith) {
    PyObject* res = PyDict_New();
    std::map<std::string, int> map = self->getVarMapFiltered(beginWith);
    std::map<std::string, int>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
      PyDict_SetItem(res, PyString_FromString(it->first.c_str()), PyInt_FromLong(it->second));
    }
    return res;
  }
  

   PyObject* getVarMapInverse() {
    PyObject* res = PyDict_New();
    std::map<int,std::string> map = self->getVarMapInverse();
    std::map<int,std::string>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
      PyDict_SetItem(res, PyInt_FromLong(it->first), PyString_FromString(it->second.c_str()));
    }
    return res;
  }
}

%extend ampls::impl::BaseCallback{
  PyObject* getSolutionDict() {
    PyObject* res = PyDict_New();
    std::vector<double> sol = self->getSolutionVector();
    std::map<int, std::string> map = self->getVarMapInverse();
    std::map<int, std::string>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
      PyDict_SetItem(res, PyString_FromString(it->second.c_str()), PyFloat_FromDouble(sol[it->first]));
    }
    return res;
  }
   PyObject* getVarMap() {
    PyObject* res = PyDict_New();
    std::map<std::string, int> map = self->getVarMap();
    std::map<std::string, int>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
      PyDict_SetItem(res, PyString_FromString(it->first.c_str()), PyInt_FromLong(it->second));
    }
    return res;
  }

   PyObject* getVarMapInverse() {
    PyObject* res = PyDict_New();
    std::map<int,std::string> map = self->getVarMapInverse();
    std::map<int,std::string>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
      PyDict_SetItem(res, PyInt_FromLong(it->first), PyString_FromString(it->second.c_str()));
    }
    return res;
  }
}


%pythoncode %{
from enum import Enum
def to_enum(enumclasses : list):
    skip = ['acquire', 'append', 'disown', 'next', 'own', 'this', 'thisown']
    for enumclass in enumclasses:
        env = {name : value for name,value in vars(enumclass).items() 
               if name not in skip and not name.startswith('__')}
        globals()[enumclass.__name__]=Enum(enumclass.__name__, env)

to_enum([Status, SolverAttributes, SolverParams, LPAlgorithms, Where, CanDo, Value, CutDirection])

# clean up the namespace
del to_enum  


def __e_to_v(v):
  if isinstance(v, Enum): return v.value
  return v

def __var_to_v(v):
    if v.type == 0:
        return v.str
    if v.type == 1:
        return v.integer
    if v.type == 2:
        return v.dbl
    raise RuntimeError("Should not happen")

def __get_ampls_parameter(self, param):
    if param.name.startswith('DBL'):
        return self.getAMPLSDoubleParameter(__e_to_v(param))
    v = self.getAMPLSIntParameter(__e_to_v(param))
    if param == SolverParams.INT_LP_Algorithm:
        return LPAlgorithms(v)
    return v

def __get_ampls_attribute(self, param):
    if param.name.startswith('DBL'):
        return self.getAMPLSDoubleAttribute(__e_to_v(param))
    return self.getAMPLSIntAttribute(__e_to_v(param))

AMPLModel.get_status=lambda self : Status(self.getStatus())
BaseCallback.get_ampls_where=lambda self : Where(self.getAMPLSWhere())
AMPLModel.set_ampls_parameter=lambda self,what,value : self.setAMPLSParameter(what.value, __e_to_v(value))
AMPLModel.get_ampls_parameter=__get_ampls_parameter
AMPLModel.get_ampls_attribute=__get_ampls_attribute

GenericCallback._getValue=GenericCallback.getValue
def _do_get_value(self, what):
    v = self._getValue(__e_to_v(what))
    return __var_to_v(v)

GenericCallback.getValue=_do_get_value
GenericCallback.get_value=_do_get_value
%}

