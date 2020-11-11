Generic Examples
================

These examples use the generic interface, which allow the same set of functions to be called on all supported solvers.

Example: load
-------------

This example shows how to load a model, solve it and display basic information.

.. literalinclude:: ../../../python/examples/load_model.py
   :language: py

Example: info callback
----------------------

This example shows how to monitor the progress of the solution process by registering a callback.

.. literalinclude:: ../../../python/examples/progress_callback.py
   :language: py


Solver specific examples
========================

Example: simple callback
------------------------

.. literalinclude:: ../../../python/examples/simple_callback_gurobi.py
   :language: py
