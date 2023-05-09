#!/bin/bash
cd "`dirname "$0"`"
curl -OLk https://ampl.com/dl/fdabrandao/solver-public-libs.zip
unzip solver-public-libs.zip
cp -r solver-public-libs/solvers-public-libs/* .
