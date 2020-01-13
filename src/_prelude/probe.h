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
