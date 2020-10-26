#default args
ARG OS_VERSION=latest
ARG INCLUDE_UHD=false
ARG INCLUDE_LIMESDR=false
ARG INCLUDE_CMNALIB_PKG=false
ARG INCLUDE_CMNALIB_GIT=true
ARG INCLUDE_SRSLTE_PKG=false
ARG INCLUDE_SRSLTE_PATCHED_PKG=false
ARG INCLUDE_SRSLTE_PATCHED_GIT=true

FROM archlinux:$OS_VERSION as archlinux_base

# Provide command add-apt-repository and apt-utils
RUN pacman -Suy --noconfirm

# General dependencies for development
RUN pacman -Sy base-devel cmake --noconfirm

# FALCON dependencies
RUN pacman -Sy git boost qt5-charts --noconfirm

# UHD (This is installed to satisfy condition RF_FOUND for srsLTE, any other supported RF frontend drivers should be good as well)
ARG INCLUDE_UHD
WORKDIR /
RUN if [ "$INCLUDE_UHD" = true ]; then \
  pacman -Sy libuhd libuhd-firmware --noconfirm \
  ; fi

ARG INCLUDE_CMNALIB_PKG
WORKDIR /
RUN if [ "$INCLUDE_CMNALIB_PKG" = true ]; then \
  git clone https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.kn.e-technik.tu-dortmund.de/falkenberg/c-mnalib-pkgbuild.git \
  && cd c-mnalib-pkgbuild \
  && makepkg -s -i -p PKGBUILD-git \
  ; fi

ARG INCLUDE_CMNALIB_GIT
WORKDIR /
RUN if [ "$INCLUDE_CMNALIB_GIT" = true ]; then \
  pacman -Sy curl glib2 systemd-libs --noconfirm \
  && git clone --branch falcon_v0.1 https://github.com/falkenber9/c-mnalib.git \
  && mkdir /c-mnalib/build \
  && cd /c-mnalib/build \
  && cmake ../ \
  && make -j install \
  ; fi

ARG INCLUDE_SRSLTE_PKG
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE_PKG" = true ]; then \
  pacman -Sy srslte --noconfirm \
  ; fi

ARG INCLUDE_SRSLTE_PATCHED_PKG
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE_PATCHED_PKG" = true ]; then \
  git clone https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.kn.e-technik.tu-dortmund.de/falkenberg/srslte-falcon-pkgbuild.git \
  && cd srslte-falcon-pkgbuild \
  && makepkg -s -i -p PKGBUILD-git \
  ; fi

ARG INCLUDE_SRSLTE_PATCHED_GIT
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE_PATCHED_GIT" = true ]; then \
  pacman -Sy fftw mbedtls boost-libs lksctp-tools libconfig pcsclite srsgui --noconfirm \
  && git clone --branch falcon_v0.1 https://github.com/falkenber9/srsLTE.git \
  && mkdir srsLTE/build \
  && cd srsLTE/build \
  && cmake -DENABLE_AVX=OFF -DENABLE_AVX2=OFF -DENABLE_FMA=OFF -DENABLE_AVX512=OFF  ../ \
  && make -j install \
  ; fi


ENTRYPOINT [ "stdbuf", "-o", "L" ]
