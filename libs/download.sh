#!/bin/bash
cd "`dirname "$0"`"
curl -O https://ampl.com/dl/fdabrandao/solvers-public-libs.zip
unzip solvers-public-libs.zip
cp -r solvers-public-libs/* .
