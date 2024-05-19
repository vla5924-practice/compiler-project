# Compiler Explorer deployment

These steps were tested on Ubuntu 22.04 and WSL.


## Install tools and Node

1. Install dependencies from apt:
   ```bash
   sudo apt update
   sudo apt install -y --no-install--recommends git build-essential python3 libssl-dev curl unzip
   ```
2. Install Node via nvm:
   ```bash
   curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
   source ~/.profile
   nvm install 20
   ```
3. Download pre-built [LLVM 17.0.6](https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.6) binaries:
   ```bash
   wget -O llvm-17.0.6.tar.xz https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04.tar.xz
   tar xf llvm-17.0.6.tar.xz
   ```


## Run an instance

1. Clone the [compiler-explorer](https://github.com/compiler-explorer/compiler-explorer) repository:
   ```bash
   git clone https://github.com/compiler-explorer/compiler-explorer.git ce
   ```
2. Copy `etc` and `lib` directories from here recursively over the repository and apply `compproj.patch`:
   ```bash
   cp -r etc ce/
   cp -r lib ce/
   cd ce
   git checkout $(cat ../commit.txt)
   git apply ../patches/compproj.patch
   ```
3. Adjust paths to the compiler and LLVM binaries in a configuration file `python.local.properties`:
   ```
   ...
   compiler.compproj.exe=/opt/compiler-project/bin/compiler
   ...
   tools.lli.exe=/opt/llvm/bin/lli
   ...
   ```
4. Build and run instance:
   ```bash
   make run EXTRA_ARGS='--language python --host 0.0.0.0 --port 10240'
   ```
