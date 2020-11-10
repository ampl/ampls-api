#!/bin/bash
cd `dirname $0`
SOLVER=cplex
PACKAGE=amplpy_cplex

mkdir -p $PACKAGE/cpp
cp -r ../../cpp/ampls $PACKAGE/cpp/ampls
cp -r ../../cpp/$SOLVER $PACKAGE/cpp/$SOLVER

mkdir -p $PACKAGE/libs
cp -r ../../libs/$SOLVER $PACKAGE/libs/$SOLVER
for p in linux64 osx64 win64; do
    mkdir -p $PACKAGE/libs/ampls/$p
    cp -r ../../libs/ampls/$p/*$SOLVER* $PACKAGE/libs/ampls/$p/
done