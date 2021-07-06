

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
}