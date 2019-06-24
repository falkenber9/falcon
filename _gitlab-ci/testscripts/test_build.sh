#!/bin/bash 

templatedir="../testdata"
builddir=$1
workdir=$2

#exit immediately if any command fails
set -e

echo "Testing download and build"

srcdir="/falcon"

mkdir -p $builddir
cd $builddir
cmake $srcdir
make -j

echo "Build successfull"
