#!/bin/bash
cd `dirname $0`
cd amplpy_copt/swig

SWIG_THREAD_FLAG=""
if [ "${AMPLPY_SWIG_THREADS:-1}" = "1" ]; then
    SWIG_THREAD_FLAG="-DUSE_PYTHON_THREADS"
fi

swig -python -c++ \
    ${SWIG_THREAD_FLAG} \
    -I../cpp/ampls/include \
    -I../cpp/copt/include \
    -I../libs/copt/include \
    amplpy_copt_swig.i
