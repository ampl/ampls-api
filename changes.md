# AMPLS-API changelog

Here **[Breaking]** means the change that may break valid user code, **[Fix]** - a bug fix.


## TBD

* **[Breaking]** ampls::CPLEXCallback is now using the generic callbacks. Note that multithreading 
  is not disabled by default but implementation needs extra care. 
  

## 0.1.2 - 2023-06-02

* **[New]** Added parameter to `AMPLModel::load()` and `AMPLAPIInterface::exportModel<T>()`
  to specify options when loading the model. Necessary for options that involve the solver 
  driver 
* **[Breaking]** Renamed `BaseCallback::checkCanDo` to `canDo`
* *Options handling*: 
  
  * **[Breaking]** Renamed the function `AMPLModel::getIntOptionValue` to `AMPLModel::getIntOption`
  * **[New]** Implemented `AMPLModel::getDoubleOption` and `AMPLModel::getStringOption`



