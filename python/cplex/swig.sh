#!/bin/bash
cd `dirname $0`
cd amplpy_cplex/swig
swig -python -c++ \
    -I../cpp/ampls/include \
    -I../cpp/cplex/include \
    -I../libs/cplex/include \
    amplpy_cplex_swig.i
