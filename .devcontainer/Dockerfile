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
    && apt-get clean autoclean \
    && apt-get autoremove -y \
    && rm -rf /var/lib/apt/lists/* \
    && ln -s $(which clang-format-15) /usr/local/bin/clang-format

RUN useradd -m user

USER user

WORKDIR /home/user

ARG LLVM_ARCHIVE_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04.tar.xz"
RUN wget -qL -O llvm.tar.xz "${LLVM_ARCHIVE_URL}" \
    && tar xzf llvm.tar.xz \
    && rm llvm.tar.xz

RUN ls -la
