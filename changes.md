# AMPLS-API changelog

Here **[Breaking]** means the change that may break valid user code, **[Fix]** - a bug fix.


## TBD

* **[Breaking]** ampls::CPLEXCallback is now using the generic callbacks. Note that multithreading 
  is not disabled by default but implementation needs extra care. 
* **[Breaking]** Solver and solver driver related errors in Python are now thrown as 
  ampls.AMPLSolverException
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



