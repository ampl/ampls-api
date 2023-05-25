#!/bin/bash
set -ex
cd "`dirname "$0"`"

for SOLVER in gurobi cplex xpress; do
    TESTS_DIR="../$SOLVER/amplpy_$SOLVER/tests/"
    rm -rf $TESTS_DIR/*
    cp *.py $TESTS_DIR
    cp -r data $TESTS_DIR
    for FILE in $TESTS_DIR/*.py; do
        sed -i~ "s/SOLVER[ \t]*=[ \t]*\"[^\"]*\"/SOLVER = \"$SOLVER\"/" $FILE
        sed -i~ "s/SOLVER[ \t]*=[ \t]*\'[^\']*\'/SOLVER = \"$SOLVER\"/" $FILE
        sed -i~ "s/amplpy_[a-z]*/amplpy_$SOLVER/" $FILE
    done
done
