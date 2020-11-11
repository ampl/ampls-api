.. _genusage:

Usage
=====

The APIs provided by ampls:: can be used in three ways (:ref:`genstructure`); each has its own advantages and drawbacks:

* **Generic usage**: using only functions available in the base classes, the same code can be reused to implement solver-independent logic. To support this type of usage, the class ``GenericCallback`` is provided (:cpp:class:`cpp <ampls::GenericCallback>`, :py:class:`python <ampls.GenericCallback>`), deriving which a solver-independent callback can be defined.
* **Solver specific usage**: using the additional functions provided by each solver implementation, the code can be used only with one solver
* **Solver native usage**: a subcase of the case above: each solver-specific model class provides access to the native C model object, on which all the C solver libraries functionalities can be used

This section will present the C++ syntax corresponding to each level, using gurobi as an example; 
the first steps (getting an instance of the :cpp:class:`ampls::GurobiModel` class) are the same for each usage type
and will not be repeated:

.. code-block:: cpp

        ampls::GurobiDrv drv = ampls::GurobiDrv()
        ampls::GurobiModel model = drv.loadModel("diet.nl")
        model.optimize()


Generic
-------

Getting the objective value relies on the function :cpp:func:`ampls::AMPLModel::getObj`, therefore the following
code is valid for each derived class (solver):

.. code-block:: cpp

        double obj = model.getObj()

Solver specific
---------------

In this case, we rely on the function :cpp:func:`ampls::GurobiModel::getDoubleAttr`, which is available only for 
gurobi:

.. code-block:: cpp

        double obj = model.getDoubleAttr(GRB_DBL_ATTR_OBJVAL)

Solver native
-------------

Here we gain access to the native C pointer and use the solver's C library to access the same value:

.. code-block:: cpp

        GRBmodel* grbm = model.getGRBmodel()
        double obj;
        int status = GRBgetdblattr(grbm, name, &obj);

If we used CPLEX, the code would be relying on the CPLEX C library:

.. code-block:: cpp
        
        // model would be an ampl::CPLEXModel here
        double obj;
        int status = CPXgetobjval(model.getCPXENV(), model.getCPXLP(), &obj);

