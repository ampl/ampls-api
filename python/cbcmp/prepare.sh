#!/bin/bash
set -ex
cd `dirname $0`
NAME=cplex
SOLVERS="cplex"
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
for s in $SOLVERS; do
    cp -r ../../cpp/$s $PACKAGE/cpp/$s
    mkdir -p $PACKAGE/libs/$s
    cp -r ../../libs/$s/include $PACKAGE/libs/$s/
    for p in $PLATFORMS; do
        mkdir -p $PACKAGE/libs/ampls/$p
        cp -r ../../libs/ampls/$p/*$s* $PACKAGE/libs/ampls/$p/
        mkdir -p $PACKAGE/libs/$s/lib/$p
        cp -r ../../libs/$s/lib/$p $PACKAGE/libs/$s/lib/
    done
done
