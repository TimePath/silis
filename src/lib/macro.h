#pragma once

#define INLINE __attribute__((always_inline)) inline

#define MACRO_BEGIN if (1) {
#define MACRO_END } else ((void) 0)

#define STATIC_INIT __attribute__ ((constructor)) static void premain(void)

#define ARRAY_LEN(x) ((sizeof(x) / sizeof((x)[0])))
