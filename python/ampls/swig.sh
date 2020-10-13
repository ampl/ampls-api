#!/bin/bash
cd `dirname $0`
cd amplpy_gurobi/swig
swig -python -c++ -builtin \
    -I../amplgurobi/ \
    -I../gurobi81/include \
    -I../ampls/include \
    amplpy_gurobi_swig.i
