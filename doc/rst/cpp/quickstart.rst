.. highlight:: bash

.. _cppquickstart:

Quickstart
==========


These sets of API are designed to work in conjunction with the AMPL solver libraries, which can 
be obtained from https://ampl.com/; these libraries provide the core functionalities of reading 
the `NL` file and writing the `sol` file and, for some solvers, allow the use of the AMPL license.

Prerequisites
-------------

To build the libraries, you must have:

- The AMPL solver shared libraries package (https://ampl.com/)
- `cmake <https://cmake.org/download/>`_ for using the provided multiplatform build system
- A build system compatible with cmake

Installation
------------

The following is a generic installation procedure; OS-specific commands are omitted.

1. Clone the solver libraries from `GitHub <https://github.com/ampl/ampls-api>`_::

        git clone https://github.com/ampl/ampls-api.git 


2. Download the solver libraries package appropriate for your platform from `here <https://ampl.com>`_
3. Extract them to ``ampls-api/libs``
4. Create a build directory in ``ampls-api``::

        cd ampls-api
        mkdir build
        cd build

5. Execute cmake::

        cmake .. -DBUILD_AMPLS=listofsolvers
   
   The currently available solvers can be looked at in the directory cpp. For example, a valid configuration
   command is::
   
        cmake .. -DBUILD_AMPLS=cplex,gurobi,xpress,copt

6. Depending on your operating system and build system, you may have a solution file or make files in the directory


Executing an example
--------------------

After a successfull build, you can find the compiled examples in the `build/bin` directory