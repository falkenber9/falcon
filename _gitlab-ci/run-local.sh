#!/bin/bash

# Helper script to run CI test locally without a gitlab runner. This file should be in sync with some pipeline from .gitlab-ci.yml.

cd docker/base-ubuntu
docker build -t base_ubuntu_srsgui --target base_ubuntu_srsgui .
cd ../../


cd docker/variants
docker build -t variant_cmnalib_uhd --build-arg INCLUDE_CMNALIB=true --build-arg INCLUDE_UHD=true .
cd ../../
#docker run -v `pwd`/../.:/falcon -v /tmp:/tmp/tmp-host -i variant_cmnalib_uhd /falcon/_gitlab-ci/testscripts/./test_all.sh ~/falcon-build /tmp
docker run -v `pwd`/../.:/falcon -v /tmp:/tmp/tmp-host -i -t variant_cmnalib_uhd /bin/bash -c "/falcon/_gitlab-ci/testscripts/./test_all.sh ~/falcon-build /tmp; bash "

#docker build \
#  --build-arg include_srsGUI=false \
#  --build-arg include_UHD=true \
#  -t falcon-test .

#docker run -v `pwd`/.:/test -i -t falcon-test bash
#docker run -v `pwd`/.:/test -i -t falcon-test /test/testscripts/./test_all.sh /falcon/build /tmp
#docker run -v `pwd`/.:/test -i -t falcon-test /bin/bash -c "/test/testscripts/./test_all.sh /falcon/build /tmp"
#docker run -v `pwd`/.:/test -i -t falcon-test /bin/bash -c "/test/testscripts/./test_all.sh /falcon/build /tmp; bash"
