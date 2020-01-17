#pragma once

// region Compiler

#define COMPILER_(x)    (1000 + (x))
#define COMPILER_CLANG  COMPILER_(1)
#define COMPILER_GCC    COMPILER_(2)
#define COMPILER_MSVC   COMPILER_(3)
#define COMPILER_TCC    COMPILER_(4)

#if 0

#elif defined(__clang__)
#define TARGET_COMPILER COMPILER_CLANG

#elif defined(__GNUC__)
#define TARGET_COMPILER COMPILER_GCC

#elif defined(_MSC_VER)
#define TARGET_COMPILER COMPILER_MSVC

#elif defined(__TINYC__)
#define TARGET_COMPILER COMPILER_TCC

#else
#error "Unknown compiler"
#endif

// endregion

// region OS

#define OS_(x)      (2000 + (x))
#define OS_OTHER    OS_(1)
#define OS_LINUX    OS_(2)
#define OS_MACOS    OS_(3)
#define OS_WINDOWS  OS_(4)

#if 0

#elif defined(__APPLE__)
#define TARGET_OS OS_MACOS

#elif defined(__EMSCRIPTEN__)
#define TARGET_OS OS_OTHER

#elif defined(__linux__)
#define TARGET_OS OS_LINUX

#elif defined(_WIN32)
#define TARGET_OS OS_WINDOWS

#else
#error "Unknown OS"
#endif

// endregion

// region Data model
#if 0

#elif defined(__SILP64__)

#define TARGET_SIZEOF_CHAR 8
#define TARGET_SIZEOF_SHORT 64
#define TARGET_SIZEOF_INT 64
#define TARGET_SIZEOF_LONG 64
#define TARGET_SIZEOF_LONGLONG 64
#define TARGET_SIZEOF_POINTER 64

#elif defined(__ILP64__)

#define TARGET_SIZEOF_CHAR 8
#define TARGET_SIZEOF_SHORT 16
#define TARGET_SIZEOF_INT 64
#define TARGET_SIZEOF_LONG 64
#define TARGET_SIZEOF_LONGLONG 64
#define TARGET_SIZEOF_POINTER 64

#elif defined(__LLP64__)

#define TARGET_SIZEOF_CHAR 8
#define TARGET_SIZEOF_SHORT 16
#define TARGET_SIZEOF_INT 32
#define TARGET_SIZEOF_LONG 32
#define TARGET_SIZEOF_LONGLONG 64
#define TARGET_SIZEOF_POINTER 64

#elif defined(__LP64__) || defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64)

#define TARGET_SIZEOF_CHAR 8
#define TARGET_SIZEOF_SHORT 16
#define TARGET_SIZEOF_INT 32
#define TARGET_SIZEOF_LONG 64
#define TARGET_SIZEOF_LONGLONG 64
#define TARGET_SIZEOF_POINTER 64

#elif defined(__ILP32__) || defined(__i386__) || defined(_M_IX86) || defined(__arm__) || defined(_M_ARM)

#define TARGET_SIZEOF_CHAR 8
#define TARGET_SIZEOF_SHORT 16
#define TARGET_SIZEOF_INT 32
#define TARGET_SIZEOF_LONG 32
#define TARGET_SIZEOF_LONGLONG 64
#define TARGET_SIZEOF_POINTER 32

#elif defined(__LP32__)

#define TARGET_SIZEOF_CHAR 8
#define TARGET_SIZEOF_SHORT 16
#define TARGET_SIZEOF_INT 16
#define TARGET_SIZEOF_LONG 32
#define TARGET_SIZEOF_LONGLONG 64
#define TARGET_SIZEOF_POINTER 32

#else
#error "Unknown data model"
#endif

// endregion
