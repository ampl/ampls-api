#!/bin/bash
set -ex
cd `dirname $0`
NAME=highs
SOLVERS="highs"
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
    cp -r ../../cpp/$s $PACKAGE/cpp/
    mkdir -p $PACKAGE/libs/$s
    cp -r ../../libs/$s/include $PACKAGE/libs/$s/
    for p in $PLATFORMS; do
        mkdir -p $PACKAGE/libs/$s/lib/$p
        cp -r ../../libs/$s/lib/$p $PACKAGE/libs/$s/lib/
        cp -r ../../libs/ampls/$p/*$s* $PACKAGE/libs/$s/lib/$p
    done
done
if [ "$#" -ne 0 ]; then
   if [ "$PLATFORMS" == "win64" ]; then 
      exit 0 
   fi
fi
os_name=$(uname -s)
# Check if the OS is Linux
if [ "$os_name" = "Linux" ]; then
    ln $PACKAGE/libs/$s/lib/linux64/libhighs.so.1 $PACKAGE/libs/$s/lib/linux64/libhighs.so
fi
if [ "$os_name" = "Darwin" ]; then
    ln $PACKAGE/libs/$s/lib/osx64/libhighs.1.dylib $PACKAGE/libs/$s/lib/osx64/libhighs.dylib
fi