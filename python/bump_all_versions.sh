#!/bin/bash
if [ "$#" -eq 0 ]; then
  echo "Usage: $0 <version>"
else
 
  version=$1
  solvers=( "ampls" "cbcmp" "copt" "cplex" "gurobi" "xpress" )
  for i in "${solvers[@]}"
  do
    cd "$i"
    ./bumpversion.sh $version
    cd ..
  done
fi




