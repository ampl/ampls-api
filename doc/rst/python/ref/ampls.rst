.. default-domain:: py

.. module:: ampls

AMPLModel
---------

.. class:: AMPLModel

   Store an in-memory representation of an AMPL model, which can be constructed by loading it from an NL file.
   It also contains two-way mappings between solver column and row numbers and AMPL entity names.
   In the following documentation, a very simple AMPL model will be used as a reference to show various function output
   values:

   .. code:: ampl

      set A{1..5};
      var x[A];
      var y <=0.5;
      maximize z: sum{a in A} x[a] + y;
      c{a in A}: x[a] <= a;

   This model is supposed to be loaded in the ampls interface via SolverDriver.loadModel() or via 
   amplpy.AMPL.exportModel()

   .. method:: getVarMapInverse() -> dict

      Get the map from variable index in the solver interface to AMPL variable instance name.  
      On the example above::
      
         model.getVarMapInverse();
         {0: 'x[1]', 1: 'x[2]', 2: 'x[3]', 3: 'x[4]', 4: 'x[5]', 5: 'y'}


   .. method:: getVarMap() -> dict

      Get the map from variable name to index in the solver interface. On the example above::

         model.getVarMap()
         {'x[1]': 0, 'x[2]': 1, 'x[3]': 2, 'x[4]': 3, 'x[5]': 4, 'y': 5}

   .. method:: getVarMapFiltered(prefix : str) -> dict

      Get the variable map filtered by the variable name, to avoid getting the whole (possibly large) map.
      The map is constructed only by the variable instances beginning with the specified string.

   .. method:: setCallback(callback : GenericCallback)

      Set a callback to be called during optimization. The callback must derive from the class GenericCallback
      and implement the method `run`, that will be called at various steps during the optimization.

   .. method:: getSolutionVector()

      Get a vector with all the variables values for the current problem::

         model.getSolutionVector()
         (1.0, 2.0, 3.0, 4.0, 5.0, 0.5)

   .. method:: getSolutionDict() -> dict

      Get a dictionary { AMPLVariableName -> value}. For the example problem::

         model.getSolutionDict()
         {'x[1]': 1.0, 'x[2]': 2.0, 'x[3]': 3.0, 'x[4]': 4.0, 'x[5]': 5.0, 'y': 0.5}

   .. method:: getNumVars()

      Get the number of variables.

   .. method:: getStatus() -> ampls.Status

     Get the solution status.

   .. method:: optimize()

      Start the optimization process.

   .. method:: writeSol(solFileName : string)

      Write the solution (in AMPL-compatible `.sol` format) to the specified file.

   .. method:: getObj()

      Get the current objective value.

   .. method:: error(code : int)

      Get the error message corresponding to the code.

   .. method:: enableLazyConstraints()

      Enable adding lazy constraints via callbacks (to be called only once)

   .. method:: printModelVars(onlyNonZero : bool)

      Utility function: prints all variables to screen.

   .. method:: getFileName() -> string

      Get the name of the NL file from which the model has been loaded from

   .. method:: setAMPLsParameter(whichParameter : ampls.SolverParams, value)

      Set a generic solver parameter to the specified value. The paramter currently mapped are
      only the ones accessible via the ampls.SolverParams enumeration. To set other solver controls
      refer to the solver specific API. Example::

         model.setAMPLsParameter(ampls.SolverParams.INT_LP_Algorithm, ampls.LPAlgorithms.Barrier)

   .. method:: getAMPLsIntParameter(whichParameter : ampls.SolverParams)

      Get the current value of an (integer) solver control. The type of the solver control is
      obvious by the enumeration name (e.g. ampls.SolverParams.INT_SolutionLimit)

   .. method:: getAMPLsDoubleParameter(whichParameter : ampls.SolverParams)

      Get the current value of a (float) solver control. The type of the solver control is
      obvious by the enumeration name (e.g. ampls.SolverParams.DBL_MIPGap)


GenericCallback
---------------

.. class:: GenericCallback

   Base abstract class for generic callbacks, inherit from this to declare a generic callback.
   Provides all mapping between solver-specific and generic values. To implement a callback, you should 
   implement the :py:meth:`GenericCallback.run` method and set it via :py:meth:`AMPLModel.setCallback` before starting the solution process 
   via :py:meth:`AMPLModel.optimize`.
   Depending on where the callback is called from, you can obtain various information about the progress 
   of the optimization and can modify the behaviour of the solver.

   .. method:: run()
      
      Function to override, called periodically by the optimizer.

   .. method:: getSolutionVector()

      Get the current solution as a vector (see :py:meth:`AMPLModel.getSolutionVector`). Note that 
      this method can not be called for all stages of the solution process, namely it can not be 
      called for all values of py:meth:`GenericCallback.getAMPLWhere()`.

   .. method:: getSolutionDict() -> dict

      Get the current solution as a dictionary (see :py:meth:`AMPLModel.getSolutionDict`). Note that 
      this method can not be called for all stages of the solution process, namely it can not be 
      called for all values of py:meth:`GenericCallback.getAMPLWhere()`.

   .. method::  getObj()
      
      Get the current objective value.  Note that 
      this method can not be called for all stages of the solution process.

   .. method::  getWhere()
   
      Get an iteger representing where in the solution process the callback has been called.
      NOTE: this is expressed using the solver’s own (not mapped) values

   .. method::  getAMPLWhere() -> ampls.Where
      
      Get where in the solution process the callback has been called (mapped). Not all possible values are mapped; in case 
      more advanced functionality is needed, please refer to the solver-specific documentation (inherit directly from 
      :py:class:`amplpy_gurobi.GurobiCallback` or :py:class:`amplpy_cplex.CPLEXCallback` depending on which solver is being used)

   .. method:: getWhereString()
      
      Get a textual representation of when in the solution process the callback has been called.

   .. method:: getMessage()
      
      Get the message that was being printed (valid is if :py:meth:`GenericCallback.getAMPLWhere` == ampls.Where.MSG)

   .. method:: getValue(Value.CBValue v)
      
      Get a (mapped) value from the solver. Not all possible values are accessible via this interface, for more advanced functionality
      refer to the solver specific documentation.

   .. method:: getVarMap()
      
      Get the map AMPLEntityName -> SolverVarIndex. See :py:meth:`AMPLModel.getVarMap`.

   .. method:: getVarMapInverse()
      
      Get the map SolverVarIndex -> AMPLEntityName. See :py:meth:`AMPLModel.getVarMapInverse`.

   .. method:: getVarMapInverse(name : string)
      
      Get the map AMPLEntityName -> SolverVarIndex for the AMPL variables which start with the specified string.
      See :py:meth:`AMPLModel.getVarMapInverse`.

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


Status
------

.. class:: Status

   These (generic) values map the most important solver statuses
   to a generic enumeration.
   Possible values are:

   .. py:data:: UNKNOWN

      Solution status unknown

   .. py:data:: OPTIMAL

      Optimal solution returned

   .. py:data:: INFEASIBLE,

      Unfeasible problem

   .. py:data:: UNBOUNDED

      Unbounded problem
      
   .. py:data:: LIMIT_ITERATION

      Hit an iterations limit
   
   .. py:data:: LIMIT_NODE

      Hit a nodes limit
   
   .. py:data:: LIMIT_TIME

      Hit a time limit
   
   .. py:data:: LIMIT_SOLUTION 

      Hit a number of solutions limit
   
   .. py:data:: INTERRUPTED

      Interrupted by the user

   .. py:data:: NOTMAPPED

      Solution status not mapped in terms of generic ampls.Status enumeration. Use the 
      solver specific API to obtain more information.


