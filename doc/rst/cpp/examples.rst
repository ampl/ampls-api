Generic Examples
================

These examples use the generic interface, which allow the same set of functions to be called on all supported solvers.

Example: load
-------------

This example shows how to load a model, solve it and display basic information.

.. literalinclude:: ../../../cpp/ampls/examples/loadModel.cpp
   :language: cpp

Example: driver error detection
-------------------------------

This example shows how to catch an error coming from the underlying driver implementation; such errors typically arise 
when specifying a wrong option when loading the model.

.. literalinclude:: ../../../cpp/ampls/examples/errorDetection.cpp
   :language: cpp


Example: information callback
-----------------------------

This example shows how to monitor the progress of the solution process by registering a callback.

.. literalinclude:: ../../../cpp/ampls/examples/getInformation.cpp
   :language: cpp


Example: add cuts in callback
-----------------------------

This example solves a traveling salesman problem by adding lazy cut to exclude subtours:

.. literalinclude:: ../../../cpp/ampls/examples/tsp.cpp
   :language: cpp


Example (requires AMPLAPI): add variables and constraints
---------------------------------------------------------

This example shows how to export a model from *AMPLAPI* to *ampls::*, how to modify it at solver interface
level by adding a variable and a constraint, optimize it and export the model back to AMPLAPI, including the 
newly defined entities.

.. literalinclude:: ../../../cpp/ampls/examples/addEntities.cpp
   :language: cpp

