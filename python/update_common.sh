#!/bin/bash
set -ex
cd "`dirname "$0"`"

version=$1
solvers=("cplex" "gurobi" "xpress" "copt" "scip")
modelclasses=("CPLEXModel" "GurobiModel" "XPRESSModel" "CoptModel" "SCIPModel") 

for index in "${!solvers[@]}"; do
    SOLVER=${solvers[index]}
    MODEL_CLASS=${modelclasses[index]}
    cp common/* "$SOLVER/amplpy_$SOLVER/"
    cp common-python-overrides.i "$SOLVER/amplpy_$SOLVER/swig/$SOLVER-python-overrides.i"
    sed -i~ "s/AMPLModel\./$MODEL_CLASS./g" "$SOLVER/amplpy_$SOLVER/swig/$SOLVER-python-overrides.i"
    TESTS_DIR="$PWD/$SOLVER/amplpy_$SOLVER/tests/"
    rm -rf $TESTS_DIR/*
    cp test/*.py $TESTS_DIR
    cp -r test/data $TESTS_DIR
    for FILE in $TESTS_DIR/*.py; do
        sed -i~ "s/SOLVER[ \t]*=[ \t]*\"[^\"]*\"/SOLVER = \"$SOLVER\"/" $FILE
        sed -i~ "s/SOLVER[ \t]*=[ \t]*\'[^\']*\'/SOLVER = \"$SOLVER\"/" $FILE
        sed -i~ "s/amplpy_[a-z]*/amplpy_$SOLVER/" $FILE
    done
done
