.. _pythonarchitecture:

Architecture
============

This section contains an overview of the architecture of the solvers API on Python; 
the classes are similar to the cpp version (shown :ref:`here <cpparchitecture>`) but are split
in separate modules.

So far, the publicly available python packages are `amplpy-gurobi <https://pypi.org/project/amplpy-gurobi/>`_ and 
`amplpy-cplex <https://pypi.org/project/amplpy-cplex>`_.

This diagram shows the available classes in three groups:

.. graphviz:: ../images/PythonStructure.dot

both amplpy-gurobi and amplpy-cplex contain the `base` group and the solver-specific classes.
In addition to the classes implemented in this library, the modules contain wrappers to the solvers' native
C library functions, useful when implementing a "solver native" :ref:`usage <genusage>`.

amplpy
------

If amplpy is installed, importing the modules add the following functions, which help creating the solver APIs 
objects and, after solving them, importing them back to AMPL:

- amplpy.AMPL.exportGurobiModel
- amplpy.AMPL.exportCplexModel
- amplpy.AMPL.exportModel(drivername)
- amplpy.AMPL.importSolution(AMPLModel)
