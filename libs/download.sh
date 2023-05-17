#!/bin/bash
cd "`dirname "$0"`"
set -ex
curl -OLk https://ampl.com/dl/fdabrandao/patch/solver-public-libs.zip
unzip solver-public-libs.zip
cp -r solver-public-libs/solver-public-libs/* .
