// Add the following for windows, otherwise it tries to link
// to python debug libraries (not always available)
%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#include <corecrt.h>
#endif
%}


%include "carrays.i"
%include <std_string.i>
%array_class(double, dblArray);
%array_class(int, intArray);

// Pass array as array and size
%typemap(in) (int nvars, const int* vars) {
  /* Check if is a list  */
  if (PyList_Check($input)) {
    int size = (int)PyList_Size($input);
    int i = 0;
    $1 = size;
    $2 = (int*)malloc(size * sizeof(int));
    for (i = 0; i < size; i++) {
      PyObject* o = PyList_GetItem($input, i);
      if (PyInt_Check(o)) {
        $2[i] = PyInt_AsLong(o);
      }
      else {
        PyErr_SetString(PyExc_TypeError, "list must contain floating-point numbers");
        free($2);
        return NULL;
      }
    }
  }
  else if ($input == Py_None) {
    $2 = NULL;
  }
  else {
    PyErr_SetString(PyExc_TypeError, "not a list");
    return NULL;
  }
}

%typemap(freearg) (int nvars, const int* vars) {
  free($2);
}

// Pass python list as double array
%typemap(in) const double* {
  /* Check if is a list  */
  if (PyList_Check($input)) {
    int size = (int)PyList_Size($input);
    int i = 0;
    $1 = (double*)malloc(size * sizeof(double));
    for (i = 0; i < size; i++) {
      PyObject* o = PyList_GetItem($input, i);
      if (PyFloat_Check(o) || PyInt_Check(o)) {
        $1[i] = PyFloat_AsDouble(o);
      }
      else {
        PyErr_SetString(PyExc_TypeError, "list must contain floating-point numbers");
        free($1);
        return NULL;
      }
    }
  }
  else if ($input == Py_None) {
    $1 = NULL;
  }
  else {
    PyErr_SetString(PyExc_TypeError, "not a list");
    return NULL;
  }
}

%typemap(freearg) (const double*) {
  free($1);
}

%typemap(in) const int* {
  /* Check if is a list  */
  if (PyList_Check($input)) {
    int size = (int)PyList_Size($input);
    int i = 0;
    $1 = (int*)malloc(size * sizeof(int));
    for (i = 0; i < size; i++) {
      PyObject* o = PyList_GetItem($input, i);
      if (PyInt_Check(o)) {
        $1[i] = PyInt_AsLong(o);
      }
      else {
        PyErr_SetString(PyExc_TypeError, "list must contain integer numbers");
        free($1);
        return NULL;
      }
    }
  }
  else if ($input == Py_None) {
    $1 = NULL;
  }
  else {
    PyErr_SetString(PyExc_TypeError, "not a list");
    return NULL;
  }
}

%typemap(freearg) (const int*) {
  free($1);
}

%include <stdint.i>

%typemap(in, numinputs = 0, noblock = 1) int* len {
  int templen;
  $1 = &templen;
}

%typemap(out)  double* getSolutionVector(int* len) {
  int i;
  $result = PyList_New(templen);
  for (i = 0; i < templen; i++) {
    PyObject* o = PyFloat_FromDouble((double)$1[i]);
    PyList_SetItem($result, i, o);
  }
  delete $1;
}

%typemap(out) myobj get(int type) {
  switch ($1.type)
  {
  case 0:
    $result = PyUnicode_FromString($1.str);
    break;
  case 1:
    $result = PyInt_FromLong($1.integer);
    break;
  case 2:
    $result = PyFloat_FromDouble($1.dbl);
    break;
  }
}

// The following are useful to return python dictionaries when not using the -builtin switch
%pythonappend ampls::AMPLModel::getSolutionDict() %{return val.asdict()%}
%pythonappend ampls::impl::BaseCallback::getSolutionDict() %{return val.asdict()%}
%ignore "getVarMap";
%ignore "getVarMapInverse";
%ignore "getVarMapFiltered";

%include "../../../exceptions-python.i"