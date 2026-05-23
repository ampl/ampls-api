#!/bin/bash
cd `dirname $0`
cd amplpy_cplex/swig

SWIG_THREAD_FLAG=""
if [ "${AMPLPY_SWIG_THREADS:-1}" = "1" ]; then
    SWIG_THREAD_FLAG="-DUSE_PYTHON_THREADS"
fi

swig -python -c++ \
    ${SWIG_THREAD_FLAG} \
    -I../cpp/ampls/include \
    -I../cpp/cplex/include \
    -I../libs/cplex/include \
    amplpy_cplex_swig.i
