#!/bin/bash 

templatedir="../testdata"
builddir=$1
workdir=$2

#exit immediately if any command fails
set -e

echo "Testing download, build and install"

srcdir="/falcon"

mkdir -p $builddir
cd $builddir
cmake -DCMAKE_INSTALL_PREFIX=/usr $srcdir
make -j

echo "Build successful"

make install

echo "Installation successful"
