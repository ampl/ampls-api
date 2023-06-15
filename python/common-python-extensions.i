

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

std::vector<Option> get_options() {
  return $self->getOptions(); }

void set_option(const char* name, int value) { 
$self->setOption(name, value); }

void set_option(const char* name, double value) { 
$self->setOption(name, value); }

void set_option(const char* name, const char* value) { 
$self->setOption(name, value); }

std::vector<Option> getOptions() {
  return $self->getOptions(); }

int get_int_option(const char* name) { 
  return $self->getIntOption(name); }

double get_double_option(const char* name) { 
  return $self->getDoubleOption(name); }

std::string get_string_option(const char* name) { 
  return $self->getStringOption(name); }
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

virtual Where::CBWhere get_ampl_where() { return $self->getAMPLWhere(); }

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


def _do_get_value(self, what):
    v = self.getValue(__e_to_v(what))
    return __var_to_v(v)

def _do_can_do(self, func):
  return self._canDo(__e_to_v(func))

def addCut(self, vars, coeffs, direction, rhs):
    return self._addCut(vars, coeffs, __e_to_v(direction), rhs)
def addLazy(self, vars, coeffs, direction, rhs):
    return self._addLazy(vars, coeffs, __e_to_v(direction), rhs)
def addCutIndices(self, nvars, coeffs, direction, rhs):
    return self._addCutIndices(nvars, coeffs, __e_to_v(direction), rhs)
def addLazyIndices(self, nvars, coeffs, direction, rhs):
    return self._addLazyIndices(nvars, coeffs, __e_to_v(direction), rhs)

# Note: do not override getValue, as it is 
# also called from the cpp routines
#BaseCallback._getValue=BaseCallback.getValue
#GenericCallback._getValue=GenericCallback.getValue
#GenericCallback.getValue=_do_get_value
#GenericCallback.get_value=_do_get_value
BaseCallback.getValue=_do_get_value
BaseCallback.get_value=_do_get_value

BaseCallback._canDo=BaseCallback.canDo
GenericCallback._canDo=GenericCallback.canDo
BaseCallback.canDo=_do_can_do
GenericCallback.canDo=_do_can_do
BaseCallback.can_do=_do_can_do
GenericCallback.can_do=_do_can_do

BaseCallback._getAMPLWhere=BaseCallback.getAMPLWhere
BaseCallback.get_ampl_where=lambda self : Where(self._getAMPLWhere())
BaseCallback.getAMPLWhere=lambda self : Where(self._getAMPLWhere())

GenericCallback._getAMPLWhere=GenericCallback.getAMPLWhere
GenericCallback.get_ampl_where=lambda self : Where(self._getAMPLWhere())
GenericCallback.getAMPLWhere=lambda self : Where(self._getAMPLWhere())


GenericCallback._addLazy=GenericCallback.addLazy
GenericCallback._addCut=GenericCallback.addCut
GenericCallback._addLazyIndices=GenericCallback.addLazyIndices
GenericCallback._addCutIndices=GenericCallback.addCutIndices
GenericCallback.addLazy=addLazy
GenericCallback.addCut=addCut
GenericCallback.addCutIndices=addCutIndices
GenericCallback.addLazyIndices=addLazyIndices
GenericCallback.add_lazy=addLazy
GenericCallback.add_cut=addCut
GenericCallback.add_cut_indices=addCutIndices
GenericCallback.add_lazy_indices=addLazyIndices
BaseCallback._addLazy=BaseCallback.addLazy
BaseCallback._addCut=BaseCallback.addCut
BaseCallback._addLazyIndices=BaseCallback.addLazyIndices
BaseCallback._addCutIndices=BaseCallback.addCutIndices
BaseCallback.addLazy=addLazy
BaseCallback.addCut=addCut
BaseCallback.addCutIndices=addCutIndices
BaseCallback.addLazyIndices=addLazyIndices
BaseCallback.add_lazy=addLazy
BaseCallback.add_cut=addCut
BaseCallback.add_cut_indices=addCutIndices
BaseCallback.add_lazy_indices=addLazyIndices

# The following are used in common-python-overrides.i
def __get_ampl_parameter(self, param):
    if param.name.startswith('DBL'):
        return self.getAMPLDoubleParameter(__e_to_v(param))
    v = self.getAMPLIntParameter(__e_to_v(param))
    if param == SolverParams.INT_LP_Algorithm:
        return LPAlgorithms(v)
    return v

def __get_ampl_attribute(self, param):
    if param.name.startswith('DBL'):
        return self.getAMPLDoubleAttribute(__e_to_v(param))
    return self.getAMPLIntAttribute(__e_to_v(param))

def __setAMPLParameter(self, what, value):
    self._setAMPLParameter(__e_to_v(what.value), value)

%}

%extend ampls::Option {
  %pythoncode %{
    def __repr__(self):
      return self.toString()
  %}
};


