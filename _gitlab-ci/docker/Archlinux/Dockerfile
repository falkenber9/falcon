#default args
ARG OS_VERSION=latest
ARG INCLUDE_UHD=false
ARG INCLUDE_LIMESDR=false
ARG INCLUDE_CMNALIB_DEPS_FOR_SUBPROJECT=false
ARG INCLUDE_CMNALIB_GIT=false
ARG INCLUDE_CMNALIB_PKG=false
ARG INCLUDE_CMNALIB=true
ARG INCLUDE_SRSLTE_PATCHED_DEPS_FOR_SUBPROJECT=false
ARG INCLUDE_SRSLTE_PATCHED_GIT=false
ARG INCLUDE_SRSLTE_PATCHED_PKG=false
ARG INCLUDE_SRSLTE_PATCHED=true
ARG INCLUDE_SRSLTE=false

FROM archlinux:$OS_VERSION as archlinux_base

# Provide command add-apt-repository and apt-utils
RUN pacman -Suy --noconfirm

# General dependencies for development
RUN pacman -Sy base-devel cmake git --noconfirm

# Additional dependencies and configure for AUR/YAY
RUN pacman -Sy go sudo --noconfirm

WORKDIR /packages
RUN useradd -d /packages packer
RUN echo "packer ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
RUN chown -R packer /packages

USER packer
RUN git clone https://aur.archlinux.org/yay.git
WORKDIR /packages/yay
RUN makepkg -si --noconfirm
USER root

# FALCON dependencies
WORKDIR /
RUN pacman -Sy boost qt5-charts --noconfirm

# UHD (This is installed to satisfy condition RF_FOUND for srsLTE, any other supported RF frontend drivers should be good as well)
ARG INCLUDE_UHD
WORKDIR /
RUN if [ "$INCLUDE_UHD" = true ]; then \
  pacman -Sy libuhd libuhd-firmware --noconfirm \
  ; fi

ARG INCLUDE_CMNALIB_DEPS_FOR_SUBPROJECT
WORKDIR /
RUN if [ "$INCLUDE_CMNALIB_DEPS_FOR_SUBPROJECT" = true ]; then \
  pacman -Sy curl glib2 systemd-libs --noconfirm \
  ; fi

ARG INCLUDE_CMNALIB_GIT
WORKDIR /
RUN if [ "$INCLUDE_CMNALIB_GIT" = true ]; then \
  pacman -Sy curl glib2 systemd-libs --noconfirm \
  && git clone https://github.com/falkenber9/c-mnalib.git \
  && mkdir /c-mnalib/build \
  && cd /c-mnalib/build \
  && cmake -DCMAKE_INSTALL_PREFIX=/usr ../ \
  && make -j install \
  ; fi

USER packer
ARG INCLUDE_CMNALIB_PKG
WORKDIR /
RUN if [ "$INCLUDE_CMNALIB_PKG" = true ]; then \
  git clone https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.kn.e-technik.tu-dortmund.de/falkenberg/c-mnalib-pkgbuild.git \
  && cd c-mnalib-pkgbuild \
  && makepkg -s -i -p PKGBUILD-git --noconfirm \
  ; fi
USER root

USER packer
ARG INCLUDE_CMNALIB
WORKDIR /
RUN if [ "$INCLUDE_CMNALIB" = true ]; then \
  yay -Sy c-mnalib --noconfirm \
  ; fi
USER root

ARG INCLUDE_SRSLTE_PATCHED_DEPS_FOR_SUBPROJECT
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE_PATCHED_DEPS_FOR_SUBPROJECT" = true ]; then \
  pacman -Sy fftw mbedtls boost-libs lksctp-tools libconfig pcsclite srsgui --noconfirm \
  ; fi

ARG INCLUDE_SRSLTE_PATCHED_GIT
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE_PATCHED_GIT" = true ]; then \
  pacman -Sy fftw mbedtls boost-libs lksctp-tools libconfig pcsclite srsgui --noconfirm \
  && git clone --branch falcon-dev https://github.com/falkenber9/srsLTE.git \
  && mkdir srsLTE/build \
  && cd srsLTE/build \
  && cmake -DENABLE_AVX=OFF -DENABLE_AVX2=OFF -DENABLE_FMA=OFF -DENABLE_AVX512=OFF  ../ \
  && make -j install \
  ; fi

USER packer
ARG INCLUDE_SRSLTE_PATCHED_PKG
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE_PATCHED_PKG" = true ]; then \
  git clone https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.kn.e-technik.tu-dortmund.de/falkenberg/srslte-falcon-pkgbuild.git \
  && cd srslte-falcon-pkgbuild \
  && makepkg -s -i -p PKGBUILD-git --noconfirm \
  ; fi
USER root

USER packer
ARG INCLUDE_SRSLTE_PATCHED
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE_PATCHED" = true ]; then \
  yay -Sy srslte-falcon-patch-git --noconfirm \
  ; fi
USER root

ARG INCLUDE_SRSLTE
WORKDIR /
RUN if [ "$INCLUDE_SRSLTE" = true ]; then \
  pacman -Sy srslte --noconfirm \
  ; fi


ENTRYPOINT [ "stdbuf", "-o", "L" ]
