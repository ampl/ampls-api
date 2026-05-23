#!/bin/bash
cd `dirname $0`
cd amplpy_cbcmp/swig

SWIG_THREAD_FLAG=""
if [ "${AMPLPY_SWIG_THREADS:-1}" = "1" ]; then
    SWIG_THREAD_FLAG="-DUSE_PYTHON_THREADS"
fi

swig -python -c++ -builtin \
    ${SWIG_THREAD_FLAG} \
    -I../cpp/ampls/include \
    -I../cpp/cbcmp/include \
    -I../libs/cbcmp/include \
    amplpy_cbcmp_swig.i
