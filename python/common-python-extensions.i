

%rename("%s") getVarMap;
%rename("%s") getVarMapInverse;
%rename("%s") getVarMapFiltered;



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

std::vector<Option>& getOptions() {
  return $self->getOptions(); }

void set_option(const char* name, int value) { 
$self->setOption(name, value); }

void set_option(const char* name, double value) { 
$self->setOption(name, value); }

void set_option(const char* name, const char* value) { 
$self->setOption(name, value); }

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

