#!/bin/bash
set -ex
cd `dirname $0`
NAME=cplexmp
SOLVER="cplex"
PACKAGE=amplpy_$NAME

PLATFORMS="linux64 osx64 win64"
if [ "$#" -ne 0 ]; then
  	PLATFORMS=$*
fi
echo "Package: $PACKAGE"
echo "Platforms: $PLATFORMS" 

rm -rf $PACKAGE/{cpp,libs}
mkdir -p $PACKAGE/cpp
cp -r ../../cpp/ampls $PACKAGE/cpp/ampls

mkdir -p $PACKAGE/libs
cp -r ../../cpp/$NAME $PACKAGE/cpp/$NAME
mkdir -p $PACKAGE/libs/$SOLVER
cp -r ../../libs/$SOLVER/include $PACKAGE/libs/$SOLVER/
for p in $PLATFORMS; do
    mkdir -p $PACKAGE/libs/ampls/$p
    cp -r ../../libs/ampls/$p/*$NAME* $PACKAGE/libs/ampls/$p/
    mkdir -p $PACKAGE/libs/$SOLVER/lib/$p
    cp -r ../../libs/$SOLVER/lib/$p $PACKAGE/libs/$SOLVER/lib/
done
