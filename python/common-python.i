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
// %pythonappend ampls::AMPLModel::getSolutionDict() %{return val.asdict()%}
// %pythonappend ampls::impl::BaseCallback::getSolutionDict() %{return val.asdict()%}
%ignore "getVarMap";
%ignore "getVarMapInverse";
%ignore "getVarMapFiltered";



%extend ampls::AMPLModel {

std::string get_recorded_entities(bool exportToAMPL = true) {
  return $self->getRecordedEntities(exportToAMPL); 
}

ampls::Constraint add_constraint(int nnz, const int* vars,
    const double* coefficients, ampls::CutDirection::Direction sense, double rhs, const char* name = NULL) {
    return  $self->addConstraint(nnz, vars, coefficients, sense, rhs, name); 
}

ampls::Variable add_variable(double lb, double ub,
    VarType::Type type, const char* name = NULL) {
    return  $self->addVariable(lb, ub, type, name);
}

ampls::Variable add_variable(int nnz, const int* cons,
      const double* coefficients, double lb, double ub, double objCoefficient,
      VarType::Type type, const char* name = NULL) {
return $self->addVariable(nnz, cons, coefficients, lb, ub, objCoefficient, type, name);      
}

std::string get_file_name() { return  $self->getFileName();}

std::map<int, std::string> get_var_map_inverse() {  return $self->getVarMapInverse();  }
std::map<int, std::string> get_con_map_inverse() {  return $self->getConMapInverse();  }

std::map<std::string, int> get_var_map() { return $self->getVarMap(); }
std::map<std::string, int> get_con_map() { return $self->getConMap(); }

std::map<std::string, int> get_var_map_filtered(const char *beginWith) { return $self->getVarMapFiltered(beginWith); }
std::map<std::string, int> get_con_map_filtered(const char *beginWith) { return $self->getConMapFiltered(beginWith); }

int set_callback(GenericCallback *callback) { return $self->setCallback(callback); }
int set_callback(impl::BaseCallback *callback) { return $self->setCallback(callback); }

std::vector<double> get_solution_vector() { return $self->getSolutionVector(); }
std::vector<double> get_dual_vector() { return $self->getDualVector(); }

virtual int get_num_vars() { return $self->getNumVars();}
virtual int get_num_cons() { return $self->getNumCons();}

virtual void write_sol() { return $self->writeSol(); }
virtual void write_sol(const char* solFileName) { return $self->writeSol(solFileName); }

virtual double get_obj() { return $self->getObj();}

virtual void enable_lazy_constraints(){ $self->enableLazyConstraints(); }

void print_model_vars(bool onlyNonZero) { $self->printModelVars(onlyNonZero); }



virtual std::vector<Option>& getOptions() {
  return $self->getOptions();
}

virtual void set_option(const char* name, int value) { $self->setOption(name, value); }

virtual void set_option(const char* name, double value) { $self->setOption(name, value); }

virtual void set_option(const char* name, const char* value) { $self->setOption(name, value); }
};

%extend ampls::impl::BaseCallback {
std::vector<double> get_solution_vector() {
  return $self->getSolutionVector();
}
virtual bool can_do(CanDo::Functionality f) {
  return $self->canDo(f); 
}
ampls::Variable add_variable(int nnz, const int* cons,
    const double* coefficients, double lb, double ub, double objCoefficient,
    VarType::Type type, const char* name = NULL) {
    return $self->addVariable(nnz, cons, coefficients, lb, ub, objCoefficient, type, name);
}
ampls::Variable add_variable(double lb, double ub,
    VarType::Type type, const char* name = NULL) {
    return $self->addVariable(lb, ub, type, name);
}
void set_debug_cuts(bool cutDebug, bool cutDebugIntCoefficients, bool cutDebugPrintVarNames) {
    $self->setDebugCuts(cutDebug, cutDebugIntCoefficients, cutDebugPrintVarNames);
}

std::map<std::string, int>& get_var_map() { return $self->getVarMap();}

std::map<int, std::string>& get_var_map_inverse() { return $self->getVarMapInverse();}

ampls::Constraint add_cut(std::vector<std::string> vars,
    const double* coeffs, CutDirection::Direction direction, double rhs) {
    return $self->addCut(vars, coeffs, direction, rhs);
}
ampls::Constraint add_lazy(std::vector<std::string> vars,
    const double* coeffs, CutDirection::Direction direction, double rhs) {
    return $self->addLazy(vars, coeffs, direction, rhs);
}

ampls::Constraint add_cut_indices(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs) {
    return $self->addCutIndices(nvars, vars, coeffs, direction, rhs);
}
ampls::Constraint add_lazy_indices(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs) {
        return $self->addLazyIndices(nvars, vars, coeffs, direction, rhs);
}
virtual int set_heuristic_solution(int nvars, const int* indices, const double* values){
    return $self->setHeuristicSolution(nvars, indices, values);
}
std::vector<double> get_solution_vector() {
  return $self->getSolutionVector();
}
virtual double get_obj() { return $self->getObj(); }

virtual int get_where() { return $self->getWhere(); }

virtual const char* get_where_string(){return $self->getWhereString(); }

virtual const char* get_message() {return $self->getMessage();}

virtual Where::CBWhere get_ampls_where() { return $self->getAMPLSWhere(); }

virtual Variant get_value(Value::CBValue v) {return $self->getValue(v);}
virtual std::vector<double> get_value_array(Value::CBValue v) {return $self->getValueArray(v);}
};
