#!/bin/bash
cd "`dirname "$0"`"
set -ex
curl -OLk https://ampl.com/dl/fdabrandao/solver-public-libs.zip
rm  -rf solver-public-libs
unzip solver-public-libs.zip
cp -r solver-public-libs/solver-public-libs/* . || cp -r solver-public-libs/* .
