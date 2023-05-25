#!/bin/bash
set -ex
cd "`dirname "$0"`"

version=$1
solvers=("cplex" "gurobi" "xpress")
for SOLVER in "${solvers[@]}"; do
    cp common/* "$SOLVER/amplpy_$SOLVER/"
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
