#!/bin/bash 

builddir=$1
workdir=$2

oldpwd=`pwd`
scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

#srcdir="/falcon"
#
#mkdir -p $builddir
#cd $builddir
#cmake $srcdir
#make -j
#
#echo "Build successfull"

cd $scriptdir

#exit immediately if any command fails
set -e

./test_build.sh $builddir $workdir
./test_FalconEye.sh $builddir $workdir
./test_imdea_cc_decoder.sh $builddir $workdir

cd $oldpwd
