.. default-domain:: py

.. module:: amplpy_gurobi

GurobiModel
-----------

.. class:: GurobiModel

   Store an in-memory representation of a gurobi model, which can be constructed by loading it from an NL file.
   It also contains two-way mappings between solver column and row numbers and AMPL entity names.
   It inherits all the members from :py:class:`ampls.AMPLModel` and adds the following:

    .. method:: getIntAttr(name : str)
        
    Get an integer model attribute (using gurobi C library name)

    .. method:: getDoubleAttr(name : str)
    Get a double model attribute (using gurobi C library name)

    .. method:: getIntAttrVector(name : str) -> list
        
    Get an integer array model attribute (using gurobi C library name)

    .. method:: getDoubleAttrVector(name : str)

    Get a double array model attribute (using gurobi C library name)

    .. method:: getIntParam(name : str)

    Get an integer parameter (using gurobi C library name)

    .. method:: getDoubleParam(name : str)

    Get a double parameter (using gurobi C library name)

    .. method:: setParam(name : str)

    Set a integer parameter (using gurobi C library name)

    .. method:: getGRBmodel() -> GRBmodel

    Get the pointer to the native C GRBmodel structure.

    .. method:: getGRBenv() -> GRBenv

    Get the pointer to the native C GRBenv structure.


GurobiCallback
--------------

.. class:: GurobiModel

   Base class for Gurobi callbacks, inherit from this to declare a callback to be called at 
   various stages of the solution process.
   Provides all mapping between solver-specific and generic values. To implement a callback, 
   you should implement the run() method and set it via :py:method:`ampls.AMPLModel.setCallback`
   before starting the solution process via :py:method:`ampls.AMPLModel.optimize`.
   Depending on where the callback is called from, you can obtain various information 
   about the progress of the optimization and can modify the behaviour of the solver.
   
   It implements all the methods in :py:class:`ampls.GenericCallback` and adds the following:

   .. method:: getCBData()
    
      Get CBdata, useful for calling gurobi c library functions.

   .. method:: getGRBModel()

      Get the underlying gurobi model pointer

   .. method:: terminate()

      Terminate the solution.

   .. method:: getInt(what : int)

      Get an integer attribute (using gurobi C library enumeration to specify what)

   .. method:: getDouble(what : int)

      Get a double attribute (using gurobi C library enumeration to specify what)

   .. method:: getDoubleArray(what : int)

      Get a double array attribute (using gurobi C library enumeration to specify what)






