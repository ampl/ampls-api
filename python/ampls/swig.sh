#!/bin/bash
cd `dirname $0`
cd amplpy_ampls/swig
swig -python -c++ -builtin \
    -I../cpp/ampls/include \
    -I../cpp/gurobi/include \
    -I../libs/gurobi/include \
    -I../cpp/cplex/include \
    -I../libs/cplex/include \
    amplpy_ampls_swig.i
