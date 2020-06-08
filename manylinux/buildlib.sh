#!/bin/bash

if [ "$#" -eq 0 ]; then
	echo "Usage: $0 [32|64]"
elif [ -d "/shared/" ]; then
    cd ~
    cp -r /src/* .
    rm -rf build
    mkdir -p build
    cd build
    export GUROBI_HOME=$HOME/amplpy_gurobi/amplpy_gurobi/gurobi81/
    export GUROBI_INCLUDE_DIR=$HOME/amplpy_gurobi/amplpy_gurobi/gurobi81/include/
    export GUROBI_LIBRARY=$HOME/amplpy_gurobi/amplpy_gurobi/gurobi81/linux64/
    cmake .. -DARCH=$1
    make all
    make package
    cp solvers.zip /shared/lib$1/
else
    echo "Must be run inside a docker container."
fi
