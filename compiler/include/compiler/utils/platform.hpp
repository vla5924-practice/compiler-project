#pragma once

#if defined(__linux__)
#define COMPILER_PLATFORM_LINUX 1
#elif defined(_WIN32)
#define COMPILER_PLATFORM_WINDOWS 1
#else
#error "Unsupported platform. Supported platforms are: Linux, Windows."
#endif

#if defined(__clang__) || defined(__llvm__)
#define COMPILER_TOOLCHAIN_CLANG 1
#define COMPILER_TOOLCHAIN_GCC_COMPATIBLE 1
#elif defined(__GNUC__)
#define COMPILER_TOOLCHAIN_GCC 1
#define COMPILER_TOOLCHAIN_GCC_COMPATIBLE 1
#elif defined(_MSC_VER)
#define COMPILER_TOOLCHAIN_MSVC 1
#else
#error "Unsupported toolchain. Supported toolchains are: clang, gcc, msvc."
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define COMPILER_ARCH_X86 1
#elif defined(__arm__) || defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARMT)
#define COMPILER_ARCH_ARM 1
#else
#error "Unsupported architecture. Supported architectures are: x86, ARM."
#endif
