.. default-domain:: py

.. module:: amplpy


AMPL
====

.. class:: AMPL

   Represents an instance of the AMPL interpreter in the AMPLAPI for Python. When importing one of the 
   ampls modules, the following functions are added:

   .. method:: exportGurobiModel(options : list = None)

      Export the current model as a GurobiModel. At this stage it is possible to pass 
      normal AMPL solver options to the solver driver, as listed by calling the AMPL solver with
      the ```-=``` parameter.
      Example: ``ampl.exportGurobiModel(["outlev=1", "return_mipgap=5"]``

      :param options: A list of options as specified to the AMPL solver driver 
      :type options: list, optional
      :return: the current model instance
      :rtype: amplpy_gurobi.GurobiModel

      
   .. method:: exportCplexModel(options : list = None)

      Export the current model as a CPLEXModel. At this stage it is possible to pass 
      normal AMPL solver options to the solver driver, as listed by calling the AMPL solver with
      the ```-=``` parameter.
      Example: ``ampl.exportCplexModel(["outlev=1", "return_mipgap=5"]``

      :param options: A list of options as specified to the AMPL solver driver
      :type options: list, optional
      :return: the current model instance
      :rtype: amplpy_cplex.CPLEXModel


   .. method:: exportModel(drivername : str, options : list = None)

      Export the current model as an ampls model. Currently the string can have
      the values `gurobi` or `cplex`, and the return type will vary accordingly.

      :param drivername: The name of the solver to export the model to (`gurobi` or `cplex`)
      :type drivername: string
      :param options: A list of options as specified to the AMPL solver driver
      :type options: list, optional

   .. method:: importSolution(model : ampls.AMPLModel)

      Import the solution represented by the AMPL model.
