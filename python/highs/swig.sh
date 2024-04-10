#!/bin/bash
cd `dirname $0`
cd amplpy_highs/swig
swig -python -c++ \
    -I../cpp/ampls/include \
    -I../cpp/highs/include \
    -I../libs/highs/include \
    amplpy_highs_swig.i
