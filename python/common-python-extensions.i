

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