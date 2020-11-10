#!/bin/bash
cd `dirname $0`
SOLVER=ampls
PACKAGE=amplpy_ampls

mkdir -p $PACKAGE/cpp
cp -r ../../cpp/ampls $PACKAGE/cpp/ampls

mkdir -p $PACKAGE/libs
for solver in gurobi cplex; do
    cp -r ../../cpp/$solver $PACKAGE/cpp/$solver
    cp -r ../../libs/$solver $PACKAGE/libs/$solver
    for p in linux64 osx64 win64; do
        mkdir -p $PACKAGE/libs/ampls/$p
        cp -r ../../libs/ampls/$p/*$solver* $PACKAGE/libs/ampls/$p/
    done
done
