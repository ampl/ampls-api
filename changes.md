# AMPLS-API changelog

Here **[Breaking]** means the change that may break valid user code, **[Fix]** - a bug fix.

## 0.2.0 - 20240327
* **[New]** Python: Added `Constraint.to_string` and `AMPLModel.add_constraint`


## 0.1.9 - 2024-03-19
* **[New]** Python: added parameter `import_entities` to the method `amplpy.AMPL.import_solution` to 
            import entities added via AMPLS back to the amplpy model
  
## 0.1.8 - 2024-03-16
* **[New]** Python: added parameter `keep_files` to the method `amplpy.AMPL.import_solution` that
            if set to true, keeps the exported row, col and NL files 
  
## 0.1.7 - 2024-01-13

* **[[Fix]]** Row and column files written to the temporary directory when exporting from AMPLAPI 
            or amplpy are now removed
            
## 0.1.6 - 2023-12-19

* **[New]** Added Python examples in python/examples
* **[Fix]** Python: `get_solution_dict` and `getSolutionDict`

## 0.1.5 - 2023-11-20

* **[Fix]** Gurobi: getting solution vector and objective bounds in callbacks at MIPSOL and MIPNODE

## 0.1.4 - 2023-08-09

* **[New]** Added SolverAttributes::INT_NumIntegerVars
* **[Fix]** Various bug fixes

## 0.1.3 - 2023-06-27

* **[Breaking]** ampls::CPLEXCallback is now using the generic callbacks. Note that multithreading 
  is not disabled by default but implementation needs extra care. 
* **[Breaking]** Solver and solver driver related errors in Python are now thrown as ampls.AMPLSolverException
* **[Fix]** Fixed python wrappers for AMPLModel.*etAMPLParameter and AMPLModel.getStatus
* **[New]** Added AMPLModel::infinity() and AMPLModel::negInfinity() to use when creating new entities
  with no bounds
* **[Fix]** A problem arising when replaying in AMPL new entities recorded with AMPLModel::record()
* **[Fix]** Mapping of callback information in BaseCallback::getValue()

## 0.1.2 - 2023-06-02

* **[New]** Added parameter to `AMPLModel::load()` and `AMPLAPIInterface::exportModel<T>()`
  to specify options when loading the model. Necessary for options that involve the solver 
  driver 
* **[Breaking]** Renamed `BaseCallback::checkCanDo` to `canDo`
* *Options handling*: 
  
  * **[Breaking]** Renamed the function `AMPLModel::getIntOptionValue` to `AMPLModel::getIntOption`
  * **[New]** Implemented `AMPLModel::getDoubleOption` and `AMPLModel::getStringOption`



