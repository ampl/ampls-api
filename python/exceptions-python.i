%include exception.i       

%exceptionclass AMPLSolverException;

%exception {
  try {
    $action
  } catch(const ampls::AMPLSolverException &e) {
      ampls::AMPLSolverException *ecopy = new ampls::AMPLSolverException(e);
      PyObject *err = SWIG_NewPointerObj(ecopy, SWIGTYPE_p_ampls__AMPLSolverException, 1);
      PyErr_SetObject(SWIG_Python_ExceptionType(SWIGTYPE_p_ampls__AMPLSolverException), err);
      SWIG_fail;
  } catch(const std::runtime_error &e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  } catch(...) {
    SWIG_exception(SWIG_RuntimeError, "Unknown exception");
  }
}
%extend  ampls::AMPLSolverException {
    const char* __str__() const {
            return $self->what();
    }
}
