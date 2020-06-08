#!/bin/bash
cd `dirname $0`

mkdir -p lib64
rm -rf lib64/*
docker run -v `pwd`:/shared -v `pwd`/../:/src --rm manylinux64 /shared/buildlib.sh 64
