.. include:: ../common/definitions.rst
.. include:: ../common/images.rst

.. _genarchitecture:

Introduction
============

|Product| is an open source set of lightweight interfaces between AMPL and solvers, which allow:

- Read in an AMPL model instance from an `NL` file
- Write out the solution as a `sol` file, ready to be imported by AMPL
- A choice between:
   - Use of all the solver's capabilities, using its own C API functionalities
   - Use of a (provided) generic interface, that encapsulates the most common 
     functionalities of the solver interfaces, permitting hassle-free solver swap
- Usage of existing AMPL licenses, when used together with the AMPL drivers

The interfaces are available for multiple languages; the core is written in C++ and it is wrapped using
`swig <https://www.swig.org>` to other target languages. 

About this documentation
========================

The documentation of this API is split in a generic part, that highlights language-agnostic concepts and a 
set of specific ones.

The (generic) introduction can be found :ref:`here <genericarchitecture>`.

Below the link to the C++ and Pyton documentations:

- :ref:`C++ <cppindex>` 
- :ref:`Python <pythonindex>` 





