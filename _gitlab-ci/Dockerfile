ARG include_srsGUI=false
ARG include_UHD=false
ARG include_limeSDR=false
ARG include_cmnalib=false

FROM ubuntu:bionic as base


# Provide command add-apt-repository and apt-utils
RUN apt-get update \
  && apt-get install -y software-properties-common apt-utils

# General dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y curl tzdata

# srsLTE dependencies
RUN apt-get update \
  && apt-get install -y build-essential git subversion cmake libboost-all-dev libboost-system-dev libboost-test-dev libboost-thread-dev libqwt-dev libqt4-dev libfftw3-dev libsctp-dev libconfig-dev libconfig++-dev libmbedtls-dev

# srsGUI and dependencies
ARG include_srsGUI
WORKDIR / 
RUN if [ "$include_srsGUI" = true ]; then \
  apt-get update \
  && apt-get install -y libboost-system-dev libboost-test-dev libboost-thread-dev libqwt-dev libqt4-dev \
  && git clone https://github.com/srsLTE/srsGUI.git \
  && mkdir /srsGUI/build \
  && cd /srsGUI/build \
  && cmake ../ \
  && make -j install \
  ; fi
# srsGUI
#RUN git clone https://github.com/srsLTE/srsGUI.git
#WORKDIR /srsGUI/build
#RUN cmake ../ \
#  && make -j install
  
# UHD (This is installed to satisfy condition RF_FOUND for srsLTE, any other supported RF frontend drivers should be good as well)
ARG include_UHD
WORKDIR / 
RUN if [ "$include_UHD" = true ]; then \
  add-apt-repository ppa:ettusresearch/uhd \
  && apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y libuhd-dev libuhd003 uhd-host \
  ; fi



# srsLTE (AVX and other accelerations are explicitly switched off. Otherwise rounding inaccuracies cause test cases to fail (if accelerated Viterbi decoder is involved in the test))
WORKDIR / 
RUN git clone --branch falcon_v0.1 https://github.com/falkenber9/srsLTE.git && echo 1 && pwd
WORKDIR /srsLTE/build
RUN cmake -DENABLE_AVX=OFF -DENABLE_AVX2=OFF -DENABLE_FMA=OFF -DENABLE_AVX512=OFF  ../ \
  && make -j install 
  
# FALCON dependencies
RUN apt-get install -y libglib2.0-dev libudev-dev libcurl4-gnutls-dev libboost-all-dev qtdeclarative5-dev libqt5charts5-dev

# FALCON	(moved into the test script, since download+build process is part of the CI test)
#WORKDIR / 
#RUN git clone https://github.com/falkenber9/falcon.git
#WORKDIR /falcon/build
#RUN cmake ../ \
#  && make -j


ENTRYPOINT [ "stdbuf", "-o", "L" ]
