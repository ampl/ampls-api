#!/bin/bash
set -ex
cd "`dirname "$0"`"
#source ../venv/bin/activate
for SOLVER in gurobi cplex xpress; do
    python -m amplpy_$SOLVER.tests
done
