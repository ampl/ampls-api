#!/bin/bash
version=$1
solvers=( "ampls" "cbcmp" "copt" "cplex" "gurobi" "xpress" )
for i in "${solvers[@]}"
do
  cp common/* "$i/amplpy_$i/"
  cp test/*.py "$i/amplpy_$i/tests/"
  find "$i/amplpy_$i/tests/" -name '*.py' -exec sed -i -e "s/amplpy_gurobi/amplpy_$i/g" {} \;
  find "$i/amplpy_$i/tests/" -name '*.py' -exec sed -i -e "s/to_ampls('gurobi')/to_ampls('$i')/g" {} \;
  cp test/data/* "$i/amplpy_$i/tests/data/"
done




