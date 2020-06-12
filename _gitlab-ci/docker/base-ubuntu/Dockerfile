
FROM ubuntu:bionic as base_ubuntu


# Provide command add-apt-repository and apt-utils
RUN apt-get update \
  && apt-get install -y software-properties-common apt-utils

# General dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y curl tzdata

# srsLTE dependencies
RUN apt-get update \
  && apt-get install -y build-essential git subversion cmake libboost-all-dev libboost-system-dev libboost-test-dev libboost-thread-dev libqwt-dev libqt4-dev libfftw3-dev libsctp-dev libconfig-dev libconfig++-dev libmbedtls-dev


FROM base_ubuntu as base_ubuntu_srsgui

WORKDIR / 
RUN apt-get update \
  && apt-get install -y libboost-system-dev libboost-test-dev libboost-thread-dev libqwt-qt5-dev qtbase5-dev \
  && git clone https://github.com/srsLTE/srsGUI.git \
  && mkdir /srsGUI/build \
  && cd /srsGUI/build \
  && cmake ../ \
  && make -j install

ENTRYPOINT [ "stdbuf", "-o", "L" ]

