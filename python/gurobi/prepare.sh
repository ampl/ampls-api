#!/bin/bash
cd `dirname $0`
mkdir -p amplpy_gurobi/{cpp,libs}

cp -r ../../cpp/ampls amplpy_gurobi/cpp/ampls
cp -r ../../cpp/gurobi amplpy_gurobi/cpp/gurobi

cp -r ../../libs/gurobi amplpy_gurobi/libs/gurobi
cp -r ../../libs/ampls amplpy_gurobi/libs/ampls
