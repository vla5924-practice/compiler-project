# Compiler

[![CI](https://github.com/vla5924-practice/compiler-project/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/vla5924-practice/compiler-project/actions/workflows/ci.yml)
[![Dev Container](https://github.com/vla5924-practice/compiler-project/actions/workflows/devcontainer.yml/badge.svg?branch=main&event=push)](https://github.com/vla5924-practice/compiler-project/actions/workflows/devcontainer.yml)
[![CodeQL](https://github.com/vla5924-practice/compiler-project/actions/workflows/codeql.yml/badge.svg?branch=main&event=push)](https://github.com/vla5924-practice/compiler-project/actions/workflows/codeql.yml)
[![Code coverage](https://coveralls.io/repos/github/vla5924-practice/compiler-project/badge.svg?branch=main)](https://coveralls.io/github/vla5924-practice/compiler-project?branch=main)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/6129/badge)](https://www.bestpractices.dev/projects/6129)

This is a home for [Compiler project](https://github.com/vla5924-practice/compiler-project), a free for use, open source, written from scratch compiler with additional support of LLVM IR code generation and binary executable production.


## Language

A compiler supports an imperative Python-flavoured language with static typing and a `main` entry function. For example, a program that calculates the factorial of an integer may be written this way:

```py
# Factorial
def fact(n: int) -> int:
    if n <= 1:
        return 1
    return n * fact(n - 1)

# Main function
def main() -> None:
    x: int = input()
    x = fact(x)
    print(x) # output
```

To learn more about the language, supported types, and constructs, visit [the docs](./docs).


## Compilation flow

The code above can be compiled to LLVM IR or even an executable file by simply running a `compiler` application:

```sh
compiler factorial.py --output factorial.ll
```

Under the hood, the code from a given input file will be processed in multiple stages using different internal representations:

* preprocessing,
* lexical and syntactic analysis (parsing),
* semantic analysis (verification),
* optimizing transformations (dead code elimination, constants propagation, etc),

and more. To learn about the overall process, frontend, and backend, visit [the docs](./docs).


## Installation

A compiler can be built from source code using [CMake](https://cmake.org/download/), as well as with the following prerequisites:

* [Ninja](https://ninja-build.org/) (this is a preferred generatior, but Unix Makefiles or MSVC++ are also supported)
* [LLVM 17.0.6](https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.6) or newer (to enable LLVM IR code generation and binary compilation via clang)

### Dev Container

Another way to obtain ready-to-go environment is pull [a Docker image](https://github.com/vla5924-practice/compiler-project/pkgs/container/compiler-project%2Fdevcontainer), then open repository in [Dev Container](https://code.visualstudio.com/docs/devcontainers/containers):

```sh
docker pull ghcr.io/vla5924-practice/compiler-project/devcontainer:latest
git clone https://github.com/vla5924-practice/compiler-project.git
devcontainer up --workspace-folder compiler-project
```


### Build with CMake

The following commands can be used to build an application and run the unit testing:

```sh
cmake -S compiler -B build -G Ninja
cmake --build build --target cli --parallel
cmake --build build --target run_tests
```

To learn more about the installation process, build options, and other features, visit [the docs](./docs).
