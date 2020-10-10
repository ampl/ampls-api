#!/bin/bash
cd `dirname $0`
cd amplpy_gurobi/swig
swig -python -c++ -builtin \
    -I../gurobi/include \
    -I../gurobi903/include \
    -I../generic/include \
    amplpy_gurobi_swig.i
