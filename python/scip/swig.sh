#!/bin/bash
cd `dirname $0`
cd amplpy_scip/swig
swig -python -c++  \
    -I../cpp/ampls/include \
    -I../cpp/scip/include \
    -I../libs/scip/include \
    amplpy_scip_swig.i
