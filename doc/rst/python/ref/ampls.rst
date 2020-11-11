.. default-domain:: py

.. module:: ampls

AMPLModel
---------

.. class:: AMPLModel

   Store an in-memory representation of an AMPL model, which can be constructed by loading it from an NL file.
   It also contains two-way mappings between solver column and row numbers and AMPL entity names.


   .. method:: getVarMapInverse()

      Get the map from variable name to index in the solver interface.


   .. method:: getVarMap()

      Get the map from variable name to index in the solver interface.

   .. method:: setCallback(callback : GenericCallback)

      Set a callback to be called during optimization.

   .. method:: getSolutionVector()

      Get all the variables of the current problem.

   .. method:: getNumVars()

      Get the number of variables.

   .. method:: getStatus()

     Get the solution status.

   .. method:: optimize()

      Start the optimization process.

   .. method:: writeSol(solFileName : string)

      Write the solution to a specific file.

   .. method:: getObj()

      Get the current objective value.

   .. method:: error(code : int)

      Get the error message corresponding to the code.

   .. method:: enableLazyConstraints()

      Enable adding lazy constraints via callbacks (to be called only once)

   .. method:: printModelVars(onlyNonZero : bool)

      Utility function: prints all variables to screen.

| 

GenericCallback
---------------

.. class:: GenericCallback

   Base abstract class for generic callbacks, inherit from this to declare a generic callback.
   Provides all mapping between solver-specific and generic values. To implement a callback, you should 
   implement the :py:meth:`GenericCallback.run` method and set it via :py:meth:`AMPLModel.setCallback` before starting the solution process 
   via :py:meth:`AMPLModel.optimize`.
   Depending on where the callback is called from, you can obtain various information about the progress 
   of the optimization and can modify the behaviour of the solver.

   .. method::  getSolution(int len, double *sol)

      Get the current solution vector

   .. method::  getObj()
      
      Get the current objective value.

   .. method::  getWhere()
   
      Get an iteger representing where in the solution process the callback has been called.
      NOTE: this is expressed using the solver’s own (not mapped) values

   .. method::  getAMPLWhere()
      
      Get where in the solution process the callback has been called (mapped)

   .. method:: getWhereString()
      
      Get a textual representation of the current solver status.

   .. method:: getMessage()
      
      Get the message that was being printed (if where == msg)

   .. method:: getValue(Value::CBValue v)
      
      Get a value from the solver.

   .. method:: run()
      
      Function to override, called periodically by the optimizer.

   .. method:: getVarMap()
      
      Get the map AMPLEntityName -> SolverVarIndex. Use as dict(getVarMap()) to have a usable Python dictionary.

   .. method:: getVarMapInverse()
      
      Get the map SolverVarIndex -> AMPLEntityName. Use as dict(getVarMapInverse()) to have a usable Python dictionary.

   .. method:: getVarMapInverse(name : string)
      
      Get the map AMPLEntityName -> SolverVarIndex for the AMPL variables which start with the specified string.
      Use as dict(getVarMapFiltered(...)) to have a usable Python dictionary.

   .. method::  addCut(vars, coeffs , direction, rhs)

         Add a user cut using AMPL variables names.

         :param list vars: List of AMPL variable names
         :param list coeffs: Vector of cut coefficients
         :param CutDirection direction: Direction of the constraint 
         :param dbl rhs: Right hand side value


   .. method::  addLazy(vars, coeffs , direction, rhs)

         Add a lazy constraint using AMPL variables names.

         :param list vars: List of AMPL variable names
         :param list coeffs: Vector of cut coefficients
         :param CutDirection direction: Direction of the constraint 
         :param dbl rhs: Right hand side value

   .. method::  addCutsIndices(nvars, vars, coeffs, direction, rhs)

         Add a user cut using solver indices.

         :param nvars: Number of variables in the cut (length of vars)
         :param list vars: Vector of variable indices (in the solvers representation)
         :param list coeffs: Vector of cut coefficients
         :param CutDirection direction: Direction of the constraint 
         :param dbl rhs: Right hand side value

   .. method::  addLazyIndices(nvars, vars, coeffs, direction, rhs)

         Add a lazy constraint using solver indices.

         :param nvars: Number of variables in the cut (length of vars)
         :param list vars: Vector of variable indices (in the solvers representation)
         :param list coeffs: Vector of cut coefficients
         :param CutDirection direction: Direction of the constraint 
         :param dbl rhs: Right hand side value

   .. method::  getSolutionVector()

   Get the current solution vector.

|

CutDirection
------------

.. class:: CutDirection

   Represent the direction of a constraint.

   .. py:data:: LE

   Less or equal

   .. py:data:: GE

   Greater or equal

| 

CBWhere
-------

.. class:: CBWhere

   These values (generic) identify where in the solution
   process a callback has been called; to get this generic value
   call GenericCallback::getAMPLType().
   Not all solvers "where" are mapped to these values; in case
   the callback is called with a not-mapped "where" parameter,
   refer to the solver-specific functionality.
 
   .. py:data:: MSG 

      When the solver wants to print a message, obtain it via GenericCallback::getMessage
    
   .. py:data:: PRESOLVE 

      Presolve phase
       
    .. py:data:: LPSOLVE 

      Executing simplex

     
    .. py:data:: MIPNODE 

      Exploring a MIP node

    .. py:data:: MIPSOL

      Found a new MIP solution

    .. py:data:: MIP

      Executing MIP algorithm

    .. py:data:: NOTMAPPED 

      Not mapped, refer to the specific user documentation




