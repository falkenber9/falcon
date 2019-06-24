
FROM base_ubuntu_srsgui:latest as base


# UHD (This is installed to satisfy condition RF_FOUND for srsLTE, any other supported RF frontend drivers should be good as well)
ARG include_UHD
WORKDIR / 
RUN if [ "$include_UHD" = true ]; then \
  add-apt-repository ppa:ettusresearch/uhd \
  && apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y libuhd-dev libuhd003 uhd-host \
  ; fi

#####################
# OTHER SDRs here
#####################



# srsLTE (AVX and other accelerations are explicitly switched off. Otherwise rounding inaccuracies cause test cases to fail (if accelerated Viterbi decoder is involved in the test))
WORKDIR / 
RUN git clone --branch falcon_v0.1 https://github.com/falkenber9/srsLTE.git && echo 1 && pwd
WORKDIR /srsLTE/build
RUN cmake -DENABLE_AVX=OFF -DENABLE_AVX2=OFF -DENABLE_FMA=OFF -DENABLE_AVX512=OFF  ../ \
  && make -j install 
  
# FALCON dependencies
RUN apt-get install -y libglib2.0-dev libudev-dev libcurl4-gnutls-dev libboost-all-dev qtdeclarative5-dev libqt5charts5-dev



ENTRYPOINT [ "stdbuf", "-o", "L" ]
