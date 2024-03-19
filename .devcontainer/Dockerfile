FROM ubuntu:22.04

ARG HTTP_PROXY=
ARG HTTPS_PROXY=
ARG http_proxy=${HTTP_PROXY}
ARG https_proxy=${HTTPS_PROXY}

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -q -y --no-install-recommends \
        cmake \
        git \
        build-essential \
        ninja-build \
        wget \
        unzip \
        ca-certificates \
        zlib1g-dev \
        libzstd-dev \
        libxml2-dev \
        sudo \
    && apt-get clean autoclean \
    && apt-get autoremove -y \
    && rm -rf /var/lib/apt/lists/* \
    && update-ca-certificates

WORKDIR /root

ARG LLVM_ARCHIVE_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04.tar.xz"
RUN mkdir llvm_install \
    && cd llvm_install \
    && wget --no-check-certificate -qL -O llvm.tar.xz "${LLVM_ARCHIVE_URL}" \
    && tar xf llvm.tar.xz \
    && rm llvm.tar.xz \
    && cp -r clang+llvm-*/* /usr/ \
    && cd .. \
    && rm -r llvm_install

RUN useradd -m user \
    && usermod -aG sudo user \
    && echo "%sudo ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER user

WORKDIR /home/user

ENTRYPOINT [ "/bin/bash" ]
