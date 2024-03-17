FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -q -y --no-install-recommends \
        cmake \
        git \
        build-essential \
        ninja-build \
        clang-format-15 \
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
RUN wget -qL -O llvm.tar.xz "${LLVM_ARCHIVE_URL}" \
    && tar xf llvm.tar.xz \
    && rm llvm.tar.xz \
    && mv clang+llvm-*/* /usr/

RUN useradd -m user \
    && usermod -aG sudo user \
    && echo "%sudo ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER user

WORKDIR /home/user

ENTRYPOINT [ "/bin/bash" ]
