.. default-domain:: py

.. module:: amplpy_cplex

The solver specific APIs give a more comprehensive access to the solver functionalities; refer to the vendor's documentation
for more information on how to use the (native) solver objects. Access to the internal native solver pointers is what 
enables access to the solver-specific functions.

CPLEXModel
----------

.. class:: CPLEXModel()

   Store an in-memory representation of a CPLEX model, which can be constructed by loading it from an NL file.
   It also contains two-way mappings between solver column and row numbers and AMPL entity names.
   It inherits all the members from :py:class:`ampls.AMPLModel` and adds the following:

   .. method:: getCPXLP() -> CPXLPptr
      
      Get the pointer to the native CPLEX LP model object.

   .. method:: getCPXENV() -> CPXENVptr
      
      Get the pointer to the native CPLEX environment object.

   .. method:: setParam(CPXPARAM : int , value : float)
      
      Set a CPLEX control parameter (using values in the CPXPARAM enumeration to specify which parameter to set)

   .. method:: getIntParam(CPXPARAM : int) -> int
      
      Get an integer CPLEX control parameter (using values in the CPXPARAM enumeration to specify which parameter to set)

   .. method:: getDoubleParam(CPXPARAM : int) -> float
      
      Get a float CPLEX control parameter (using values in the CPXPARAM enumeration to specify which parameter to set)


CPLEXCallback
-------------

   Base class for CPLEX callbacks, inherit from this to declare a callback to be called at 
   various stages of the solution process.
   Provides all mapping between solver-specific and generic values. To implement a callback, 
   you should implement the run() method and set it via :py:method:`ampls.AMPLModel.setCallback`
   before starting the solution process via :py:method:`ampls.AMPLModel.optimize`.
   Depending on where the callback is called from, you can obtain various information 
   about the progress of the optimization and can modify the behaviour of the solver.
   
   It implements all the methods in :py:class:`ampls.GenericCallback` and adds the following:

   .. method:: getInt(what : int)
      
      Get an integer value at this stage of the solution process. See the CPX_CALLBACK_ enumerations for possible values for the parameter what.

   .. method:: getDouble(int what)
   
      Get a double value at this stage of the solution process. See the CPX_CALLBACK_ enumerations for possible values for the parameter what.

