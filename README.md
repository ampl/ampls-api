# AMPLS-API

[![Build Status](https://dev.azure.com/ampldev/ampls-api/_apis/build/status/ampl.ampls-api?branchName=master)](https://dev.azure.com/ampldev/ampls-api/_build/latest?definitionId=9&branchName=master)

AMPLS-API is an open source set of lightweight interfaces between AMPL and solvers, which allow:

- Read in an AMPL model instance from an `NL` file
- Write out the solution as a `sol` file, ready to be imported by AMPL
- A choice between:
   - Use of all the solver's capabilities, using its own C API functionalities
   - Use of a (provided) generic interface, that encapsulates the most common 
     functionalities of the solver interfaces, permitting hassle-free solver swap
- Usage of existing AMPL licenses, when used together with the AMPL drivers

The interfaces are available for multiple languages; the core is written in C++ and it is wrapped using
[swig](https://www.swig.org) to other target languages. 

## Documentation

- https://ampls.readthedocs.io/

## Solvers

- Gurobi
- CPLEX
- XPRESS 

## Languages

### Python

#### Examples

- Example: [Jupyter Notebook with TSP Example](python/examples/notebooks/tsp_simple_cuts_generic.ipynb)

- More examples:
  - [python/examples/](python/examples/)
  - [python/examples/notebooks/](python/examples/notebooks/)

#### Repositories

- PyPI Repository for amplpy-gurobi: https://pypi.python.org/pypi/amplpy-gurobi
- PyPI Repository for amplpy-cplex: https://pypi.python.org/pypi/amplpy-cplex

#### Setup

##### PyPI

Install from the PyPI repositories:
```
$ python -m pip install amplpy-gurobi amplpy-cplex
```

Note: For Windows, Linux, and macOS, the python packages come with binary wheels for many Python versions and platforms. Please make sure that you are using the latest version of `pip` before installing them (upgrade using `pip install pip --upgrade` or `python -m pip install pip --upgrade`). If a binary wheel for your platform or python version is not available, a C++ compiler and python development libraries will be required.

## License

BSD-3

***
Copyright © 2020-2021 AMPL Optimization inc. All rights reserved.
