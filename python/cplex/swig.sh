#!/bin/bash
cd `dirname $0`
cd amplpy_cplex/swig
swig -python -c++ -builtin \
    -I../cpp/generic/include \
    -I../cpp/cplex/include \
    -I../libs/cplex/include \
    amplpy_cplex_swig.i
