# Compiler Explorer deployment

1. Clone [compiler-explorer](https://github.com/compiler-explorer/compiler-explorer) repository:
   ```bash
   git clone https://github.com/compiler-explorer/compiler-explorer.git ce
   ```
1. Copy `etc` and `lib` directories from here recursively over the repository and apply `compproj.patch`:
   ```bash
   cp -r etc ce/
   cp -r lib ce/
   cd ce
   git apply ../patches/compproj.patch
   ```
1. Adjust paths to the compiler binaries and clang tools in a configuration file.
1. Build and run instance:
   ```bash
   make run EXTRA_ARGS='--language c++,python --host 0.0.0.0 --port 10240'
   ```
