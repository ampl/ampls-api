#!/bin/bash
cd `dirname $0`

docker pull quay.io/pypa/manylinux1_x86_64

docker rm manylinux64 manylinux64_tmp
docker run -v `pwd`:/manylinux --name manylinux64_tmp quay.io/pypa/manylinux1_x86_64 /manylinux/bootstrap.sh
docker commit manylinux64_tmp manylinux64
docker rm manylinux64_tmp
