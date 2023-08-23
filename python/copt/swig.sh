#!/bin/bash
cd `dirname $0`
cd amplpy_copt/swig
swig -python -c++ \
    -I../cpp/ampls/include \
    -I../cpp/copt/include \
    -I../libs/copt/include \
    amplpy_copt_swig.i
