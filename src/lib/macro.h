#pragma once

#define INLINE __attribute__((always_inline)) inline

#define MACRO_BEGIN if (1) {
#define MACRO_END } else ((void) 0)

#define STATIC_INIT(name) __attribute__ ((constructor)) static void CAT2(premain_, name)(void)

#define ARRAY_LEN(x) ((sizeof(x) / sizeof((x)[0])))

#define CAT2(_0, _1) _0 ## _1
