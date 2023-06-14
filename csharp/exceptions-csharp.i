%include exception.i       

%exception {
  try {
    $action
  } catch(const ampls::AMPLSolverException &e) {
     SWIG_exception(SWIG_RuntimeError, e.what());
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
