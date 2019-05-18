#pragma once

#include "macro.h"

#define Result(T, E) CAT4(Result__, T, __, E)
#define Result_instantiate(T, E) typedef Result_(T, E) Result(T, E)
#define Result_(T, E) \
struct { \
    union { bool ok; size_t _ok; }; \
    union { \
        T val; \
        E err; \
    }; \
} \
/**/

#define Result_ok(it) { .ok = true, .val = (it), }
#define Result_err(it) { .ok = false, .err = (it), }
