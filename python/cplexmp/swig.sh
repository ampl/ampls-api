#!/bin/bash
cd `dirname $0`
cd amplpy_cplexmp/swig
swig -python -c++ \
    -I../cpp/ampls/include \
    -I../cpp/cplexmp/include \
    -I../libs/cplex/include \
    amplpy_cplexmp_swig.i
