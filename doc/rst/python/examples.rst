
Example: callbacks_first
-------------------------

Demonstrates the use of generic callbacks

.. literalinclude:: ../../../python/examples/callbacks_first.py
   :language: py


Example: callbacks_stopping
----------------------------

Stopping the solution process given a certain criteria
using callbacks

.. literalinclude:: ../../../python/examples/callbacks_stopping.py
   :language: py


Example: load_model
--------------------

Shows how to export a model from amplpy to ampls,
how to solve it and how to get the solution as a vector or
as a dictionary

.. literalinclude:: ../../../python/examples/load_model.py
   :language: py


Example: multiple_models
-------------------------

Shows how to create multiple models with AMPL, export an AMPLs instance
for a specific solver, set some solver options and get status/solution, and get the results
back in AMPL. It also shows how to use the native C API of various solvers

.. literalinclude:: ../../../python/examples/multiple_models.py
   :language: py


Example: options
-----------------

How to set options in the solver driver using:
1) solver driver options 
2) solver specific parameters

.. literalinclude:: ../../../python/examples/options.py
   :language: py


Example: pattern_generation
----------------------------

Solves a cutting stock problem using two AMPL problems for pattern generation
and shows how to terminate the solution of the final MIP with a callback by
increasing the allowed MIP gap depending on how many solutions have been found

.. literalinclude:: ../../../python/examples/pattern_generation.py
   :language: py


Example: tsp_callback
----------------------

This example uses generic callbacks to solve a travelling salesperson problem 
with MTZ cuts or with sub-tour elimination constraints.

.. literalinclude:: ../../../python/examples/tsp_callback.py
   :language: py

