.. include:: ../../common/definitions.rst


Introduction
============

|Product| is an open source set of lightweight interfaces between AMPL and solvers, which allow:

- Read in an AMPL model instance from an `NL` file
- Write out the solution as a `sol` file, ready to be imported by AMPL
- A choiche between:
   - Use of all the solver's capabilities, using its own C API functionalities
   - Use of a (provided) generic interface, that encapsulates the most common 
     functionalities of the solver interfaces, permitting hassle-free solver swap
- Usage of existing AMPL licenses, when used together with the AMPL drivers



