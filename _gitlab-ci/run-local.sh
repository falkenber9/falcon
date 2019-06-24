#!/bin/bash

# Helper script to run CI test locally without a gitlab runner. This files should be in sync with the .gitlab-ci.yml contents.

docker build \
  --build-arg include_srsGUI=false \
  --build-arg include_UHD=true \
  -t falcon-test .

#docker run -v `pwd`/.:/test -i -t falcon-test bash
#docker run -v `pwd`/.:/test -i -t falcon-test /test/testscripts/./test_all.sh /falcon/build /tmp
docker run -v `pwd`/.:/test -i -t falcon-test /bin/bash -c "/test/testscripts/./test_all.sh /falcon/build /tmp"
#docker run -v `pwd`/.:/test -i -t falcon-test /bin/bash -c "/test/testscripts/./test_all.sh /falcon/build /tmp; bash"
