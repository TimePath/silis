#pragma once

// region Compiler

#if 0

#elif defined(__clang__)
#define TARGET_COMPILER_CLANG 1
#define TARGET_COMPILER_GCC 0
#define TARGET_COMPILER_MSVC 0
#define TARGET_COMPILER_TCC 0

#elif defined(__GNUC__)
#define TARGET_COMPILER_CLANG 0
#define TARGET_COMPILER_GCC 1
#define TARGET_COMPILER_MSVC 0
#define TARGET_COMPILER_TCC 0

#elif defined(_MSC_VER)
#define TARGET_COMPILER_CLANG 0
#define TARGET_COMPILER_GCC 0
#define TARGET_COMPILER_MSVC 1
#define TARGET_COMPILER_TCC 0

#elif defined(__TINYC__)
#define TARGET_COMPILER_CLANG 0
#define TARGET_COMPILER_GCC 0
#define TARGET_COMPILER_MSVC 0
#define TARGET_COMPILER_TCC 1

#else
#error "Unknown compiler"
#endif

// endregion

// region OS

#if 0

#elif defined(__APPLE__)
#define TARGET_OS_LIN 0
#define TARGET_OS_MAC 1
#define TARGET_OS_WIN 0

#elif defined(__EMSCRIPTEN__)
#define TARGET_OS_LIN 0
#define TARGET_OS_MAC 0
#define TARGET_OS_WIN 0

#elif defined(__linux__)
#define TARGET_OS_LIN 1
#define TARGET_OS_MAC 0
#define TARGET_OS_WIN 0

#elif defined(_WIN32)
#define TARGET_OS_LIN 0
#define TARGET_OS_MAC 0
#define TARGET_OS_WIN 1

#else
#error "Unknown OS"
#endif

// endregion
