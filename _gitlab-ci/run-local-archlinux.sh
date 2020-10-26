#!/bin/bash

# Helper script to run CI test locally without a gitlab runner. This file should be in sync with some pipeline from .gitlab-ci.yml.

docker rmi archlinux_base:latest || true
cd docker/Archlinux

BUILD_PARAMS=(
#	--no-cache
	-t archlinux_base:latest
	--target archlinux_base
	--build-arg OS_VERSION=latest
	--build-arg INCLUDE_UHD=false
	--build-arg INCLUDE_LIMESDR=false
	--build-arg INCLUDE_CMNALIB_PKG=false
	--build-arg INCLUDE_CMNALIB_GIT=true
	--build-arg INCLUDE_SRSLTE_PKG=false
	--build-arg INCLUDE_SRSLTE_PATCHED_PKG=false
	--build-arg INCLUDE_SRSLTE_PATCHED_GIT=true
	.
)
docker build ${BUILD_PARAMS[@]}

cd ../../

RUN_PARAMS=(
	-v `pwd`/../.:/falcon
	-v /tmp:/tmp/tmp-host
	-i
	-t archlinux_base:latest
)
echo ${RUN_PARAMS[@]}
docker run ${RUN_PARAMS[@]} /bin/bash -c "/falcon/_gitlab-ci/testscripts/./test_all.sh /falcon-build /tmp; bash"
