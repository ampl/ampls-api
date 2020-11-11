.. default-domain:: py

.. module:: ampls

AMPLModel
=========

.. class:: AMPLModel

   Store an in-memory representation of an AMPL model, which can be constructed by loading it from an NL file.
   It also contains two-way mappings between solver column and row numbers and AMPL entity names.


.. function:: AMPLModel.optimize()

   Solves a model

.. function:: AMPLModel.getObj()

   Get the objective function value
