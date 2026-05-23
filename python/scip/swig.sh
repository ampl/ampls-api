#!/bin/bash
cd `dirname $0`
cd amplpy_scip/swig

SWIG_THREAD_FLAG=""
if [ "${AMPLPY_SWIG_THREADS:-1}" = "1" ]; then
    SWIG_THREAD_FLAG="-DUSE_PYTHON_THREADS"
fi

swig -python -c++  \
    ${SWIG_THREAD_FLAG} \
    -I../cpp/ampls/include \
    -I../cpp/scip/include \
    -I../libs/scip/include \
    amplpy_scip_swig.i
