#!/bin/bash
cd `dirname $0`
mkdir -p amplpy_cplex/{cpp,libs}

cp -r ../../cpp/ampls amplpy_cplex/cpp/ampls
cp -r ../../cpp/cplex amplpy_cplex/cpp/cplex

cp -r ../../libs/cplex amplpy_cplex/libs/cplex
cp -r ../../libs/ampls amplpy_cplex/libs/ampls
 
