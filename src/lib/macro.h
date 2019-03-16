#pragma once

#if defined(_MSC_VER)
#define INLINE inline
#else
#define INLINE __attribute__((always_inline)) inline
#endif

#define MACRO_BEGIN if (1) {
#define MACRO_END } else ((void) 0)

#define ARRAY_LEN(x) ((sizeof(x) / sizeof((x)[0])))

#define CAST(T, U, it) ((union { U from; T to; }) { .from = (it) }.to)

#define CAT2(_0, _1) _0 ## _1
#define CAT3(_0, _1, _2) _0 ## _1 ## _2

#define STRINGIFY(self) #self

#if defined(__clang__)
#define DIAG_PUSH         _Pragma("GCC diagnostic push")
#define DIAG_IGNORE(rule) _Pragma(STRINGIFY(GCC diagnostic ignored rule))
#define DIAG_POP          _Pragma("GCC diagnostic pop")
#else
#define DIAG_PUSH
#define DIAG_IGNORE(rule)
#define DIAG_POP
#endif

#if defined(__has_warning)
#if __has_warning("-Wredundant-parens")
#define DIAG_IGNORE_REDUNDANT_PARENS DIAG_IGNORE("-Wredundant-parens")
#endif
#endif

#ifndef DIAG_IGNORE_REDUNDANT_PARENS
#define DIAG_IGNORE_REDUNDANT_PARENS
#endif
