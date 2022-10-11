#!/bin/bash
cd `dirname $0`
cd amplpy_gurobi/swig
swig -python -c++ -builtin \
    -I../cpp/ampls/include \
    -I../cpp/xpress/include \
    -I../libs/xpress/include \
    amplpy_xpress_swig.i
