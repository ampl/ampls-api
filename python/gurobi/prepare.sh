#!/bin/bash
cd `dirname $0`
cp -r ../../cpp/generic amplpy_gurobi/
cp -r ../../cpp/gurobi amplpy_gurobi/
cp -r ../../../solver-libraries/gurobi/903 amplpy_gurobi/gurobi903
cp ../../../solvers-private/build/lib/{libgurobi-drv.a,libsimpleapi.a} amplpy_gurobi/ # FIXME: make cross platform
cp ../../../solvers-private/build/bin/libgurobi-lib.dylib amplpy_gurobi/ # FIXME: make cross platform
