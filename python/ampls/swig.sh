#!/bin/bash
cd `dirname $0`
cd amplpy_ampls/swig
SWIG_THREAD_FLAG=""
if [ "${AMPLPY_SWIG_THREADS:-1}" = "1" ]; then
    SWIG_THREAD_FLAG="-DUSE_PYTHON_THREADS"
fi
swig -python -c++ -builtin \
    ${SWIG_THREAD_FLAG} \
    -I../cpp/ampls/include \
    -I../cpp/gurobi/include \
    -I../libs/gurobi/include \
    -I../cpp/cplex/include \
    -I../libs/cplex/include \
    amplpy_ampls_swig.i
