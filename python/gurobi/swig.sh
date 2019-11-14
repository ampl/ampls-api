#!/bin/bash
cd `dirname $0`
cd amplpy_gurobi/swig
swig -python -c++ -builtin \
    -I../amplgurobi/ \
    -I../gurobi81/include \
    -I../simpleapi/include \
    amplpy_gurobi_swig.i
