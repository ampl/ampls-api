.. default-domain:: py

.. module:: amplpy


AMPL
====

.. class:: AMPL

   Represents an instance of the AMPL interpreter in the AMPLAPI for Python. When importing one of the 
   ampls modules, the following functions are added:

   .. method:: exportGurobiModel()

      Export the current model as a GurobiModel 

      :return: the current model instance
      :rtype: amplpy_gurobi.GurobiModel

   .. method:: exportCplexModel()

      Export the current model as a CPLEXModel 

      :return: the current model instance
      :rtype: amplpy_cplex.CPLEXModel


   .. method:: exportModel(drivername : str)

      Export the current model as an ampls model. The string can be "gurobi" or "cplex", and the return
      type will vary accordingly.


   .. method:: importSolution(model : ampls.AMPLModel)

      Import the solution represented by the AMPL model
