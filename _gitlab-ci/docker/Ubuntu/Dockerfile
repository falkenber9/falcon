#default args
ARG OS_VERSION=latest
ARG INCLUDE_SRSGUI=true
ARG INCLUDE_UHD=false
ARG INCLUDE_LIMESDR=false
ARG INCLUDE_CMNALIB=false
ARG INCLUDE_SRSLTE=true

FROM ubuntu:$OS_VERSION as ubuntu_base


# Provide command add-apt-repository and apt-utils
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update \
  && apt-get install -y software-properties-common apt-utils

# General dependencies
#RUN apt-get install -y curl tzdata

# Build dependencies
RUN apt-get install -y build-essential git

# srsLTE dependencies
RUN apt-get update \
  && apt-get install -y cmake libfftw3-dev libmbedtls-dev libboost-program-options-dev libconfig++-dev libsctp-dev
  #build-essential git cmake libboost-all-dev libboost-system-dev libboost-test-dev libboost-thread-dev libfftw3-dev libsctp-dev libconfig-dev libconfig++-dev libmbedtls-dev
  #build-essential git subversion cmake libboost-all-dev libboost-system-dev libboost-test-dev libboost-thread-dev libqwt-dev libqt4-dev libfftw3-dev libsctp-dev libconfig-dev libconfig++-dev libmbedtls-dev

# SRS GUI
ARG INCLUDE_SRSGUI
WORKDIR / 
RUN if [ "$INCLUDE_SRSGUI" = true ]; then \
  apt-get update \
  && apt-get install -y libboost-system-dev libboost-test-dev libboost-thread-dev libqwt-qt5-dev qtbase5-dev \
  && git clone https://github.com/srsLTE/srsGUI.git \
  && mkdir /srsGUI/build \
  && cd /srsGUI/build \
  && cmake ../ \
  && make -j install \
  ; fi

# UHD (This is installed to satisfy condition RF_FOUND for srsLTE, any other supported RF frontend drivers should be good as well)
ARG INCLUDE_UHD
WORKDIR / 
RUN if [ "$INCLUDE_UHD" = true ]; then \
  add-apt-repository ppa:ettusresearch/uhd \
  && apt-get update \
  && apt-get install -y libuhd-dev uhd-host \
  ; fi

#libuhd-dev libuhd003 uhd-host

# LimeSDR
#ARG INCLUDE_LIMESDR
#WORKDIR / 
#RUN if [ "$INCLUDE_LIMESDR" = true ]; then \
#  ...
#  ; fi

# CMNALIB
ARG INCLUDE_CMNALIB
WORKDIR / 
RUN if [ "$INCLUDE_CMNALIB" = true ]; then \
  apt-get update \
  && apt-get install -y libglib2.0-dev libudev-dev libcurl4-gnutls-dev \
  && git clone https://github.com/falkenber9/c-mnalib.git \
  && mkdir /c-mnalib/build \
  && cd /c-mnalib/build \
  && cmake ../ \
  && make -j install \
  ; fi

# srsLTE (AVX and other accelerations are explicitly switched off. Otherwise rounding inaccuracies cause test cases to fail (if accelerated Viterbi decoder is involved in the test))
ARG INCLUDE_SRSLTE
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE" = true ]; then \
  git clone --branch falcon-dev https://github.com/falkenber9/srsLTE.git \
  && mkdir srsLTE/build \
  && cd srsLTE/build \
  && cmake -DENABLE_AVX=OFF -DENABLE_AVX2=OFF -DENABLE_FMA=OFF -DENABLE_AVX512=OFF  ../ \
  && make -j install \
  ; fi

# FALCON dependencies
RUN apt-get update && apt-get install -y libglib2.0-dev libudev-dev libcurl4-gnutls-dev libboost-all-dev qtdeclarative5-dev libqt5charts5-dev

ENTRYPOINT [ "stdbuf", "-o", "L" ]
