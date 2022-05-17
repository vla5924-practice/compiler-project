FROM ubuntu:20.04 as build

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y --no-install-recommends cmake build-essential libncurses5-dev python3 zlib1g-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/build

ARG LLVM_BINARIES_ARCHIVE=https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
ADD ${LLVM_BINARIES_ARCHIVE} /opt/build
RUN cd /opt/build && \
    ln -s ./$(ls) llvm

ENV LD_LIBRARY_PATH=/opt/build/llvm/lib:$LD_LIBRARY_PATH
ENV LLVM_DIR=/opt/build/llvm/lib/cmake/llvm

RUN mkdir -p /opt/build/compiler-project
COPY . /opt/build/compiler-project

ARG ENABLE_TESTS=ON
RUN cd /opt/build/compiler-project && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=${ENABLE_TESTS} -DENABLE_IR_GENERATOR=ON .. && \
    make -j

FROM ubuntu:20.04 as prod

RUN useradd -ms /bin/bash user
USER user

RUN mkdir -p /home/user/llvm/bin && mkdir -p /home/user/llvm/lib
COPY --from=build --chown=user /opt/build/llvm/bin/clang-13 /opt/build/llvm/bin/llc /home/user/llvm/bin/
COPY --from=build --chown=user /opt/build/llvm/lib/libc++*  /opt/build/llvm/lib/libclang.so* /opt/build/llvm/lib/libLLVMCore.a /home/user/llvm/lib/
RUN cd /home/user/llvm/bin && \
    ln -s ./clang-13 clang && \
    ln -s ./clang-13 clang++

RUN mkdir -p /home/user/compiler/bin && \
    mkdir -p /home/user/compiler/workspace
COPY --from=build --chown=user /opt/build/compiler-project/build/bin /home/user/compiler/bin

ENV PATH=/home/user/compiler/bin:/home/user/llvm/bin:$PATH
ENV LD_LIBRARY_PATH=/home/user/llvm/lib:$LD_LIBRARY_PATH

WORKDIR /home/user/compiler/workspace

ENTRYPOINT [ "/home/user/compiler/bin/cli" ]
CMD [ "--help" ]
