#!/bin/bash
cd `dirname $0`
cd amplpy_gurobi/swig
swig -python -c++ -builtin \
    -I../cpp/ampls/include \
    -I../cpp/gurobi/include \
    -I../libs/gurobi/include \
    amplpy_gurobi_swig.i
