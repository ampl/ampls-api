#!/bin/bash
cd `dirname $0`
cd amplpy_gurobi/swig

SWIG_THREAD_FLAG=""
if [ "${AMPLPY_SWIG_THREADS:-1}" = "1" ]; then
    SWIG_THREAD_FLAG="-DUSE_PYTHON_THREADS"
fi

swig -python -c++  \
    ${SWIG_THREAD_FLAG} \
    -I../cpp/ampls/include \
    -I../cpp/gurobi/include \
    -I../libs/gurobi/include \
    amplpy_gurobi_swig.i
