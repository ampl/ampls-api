#!/bin/bash

if [ -z "$1" ]; then
  echo "SOLVER argument not provided. Exiting..."
  exit 1
fi

if [ "$1" = "all" ]; then
  SOLVERS=("gurobi" "cplex" "xpress")
else
  SOLVERS=("$1")
fi

for s in "${SOLVERS[@]}"; do
  cd "$s"
  ./prepare.sh
  pip install -e .
  python -m amplpy_"$s".tests
  cd ..
done
