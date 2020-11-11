.. _genstructure:

Structure
=========

The API is written in C++ (code in `/cpp`) and is then wrapped to other target languages
by using SWIG. Therefore, any modification, fix, or the code to add a solver API must
be written in such language and follow its structure.

The interfaces are intented to be very simple, to encourage people contributions and to ease debugging.
In a nutshell, they are organised as a (core) set of interfaces and some derived implementations; specifically,
each solver must implement three interfaces, that are represented in C++
by three abstract classes.

It has to be noted that, since the AMPLModel-derived object instance contains a reference
to the solver-specific in memory representation of the model, there is no possible functionality
to create a model instance linked to one solver and solve it with another one.


The following are the base classes implemented for each solver:

.. graphviz:: ../images/GenericStructure.dot

* ``AMPLModel`` (:cpp:class:`cpp <ampls::AMPLModel>`, :py:class:`python <ampls.AMPLModel>`) is the main interface, it provides functionalities like:
 
  * keep a reference to the in-memory representation of the model
  * solve the model
  * get solution status, solution vector and objective value
  * access common properties like number of variables, of constraints
  * assign callback object

* ``BaseCallback`` when derived for a specific solver (see for example :cpp:class:`cpp <ampls::CPLEXCallback>` and :py:class:`python <ampls::CPLEXCallback>`), provides:

  * access to various properties accessible during the solution process
  * functions to add cuts or lazy constraints
  * ability to terminate the solution process

So, taking gurobi as an example, we have:

.. graphviz:: ../images/gurobiStructure.dot

The derived classes implement the interfaces specified in their base classes
and extend them with some solver-specific functionalities. This leads to three possible
usages:

* **Generic usage**: using only functions available in the base classes, the same code can be reused to implement solver-independent logic. To support this type of usage, the class ``GenericCallback`` is provided (:cpp:class:`cpp <ampls::GenericCallback>`, :py:class:`python <ampls.GenericCallback>`), deriving which a solver-independent callback can be defined.
* **Solver specific usage**: using the additional functions provided by each solver implementation, the code can be used only with one solver
* **Solver native usage**: a subcase of the case above: each solver-specific model class provides access to the native C model object, on which all the C solver libraries functionalities can be used


