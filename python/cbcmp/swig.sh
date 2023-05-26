#!/bin/bash
cd `dirname $0`
cd amplpy_cbcmp/swig
swig -python -c++ -builtin \
    -I../cpp/ampls/include \
    -I../cpp/cbcmp/include \
    -I../libs/cbcmp/include \
    amplpy_cbcmp_swig.i
