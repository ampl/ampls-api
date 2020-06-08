#!/bin/bash

BUILD_SWIG=FALSE

cd /opt/
TGZ=cmake-3.6.3-Linux-x86_64.tar.gz
curl -O https://cmake.org/files/v3.6/$TGZ
tar xzvf $TGZ
rm -rf $TGZ
DIR=`basename $TGZ .tar.gz`
for binary in `pwd`/$DIR/bin/*; do
    ln -s $binary /usr/local/bin/
done;
cmake --version

if [ $BUILD_SWIG = "TRUE" ]; then
    SWIG_VERSION=4.0.1
    cd /opt/
    TGZ=swig-$SWIG_VERSION.tar.gz
    curl -LO https://prdownloads.sourceforge.net/swig/$TGZ
    tar xzvf $TGZ
    rm -rf $TGZ
    DIR=`basename $TGZ .tar.gz`
    cd $DIR
    ./configure --without-pcre
    make -j4
    make install
    swig -version
fi
