#pragma once

#include "macro.h"

#define Result(T, E) CAT4(Result__, T, __, E)
#define Result_instantiate(T, E) typedef Result_(T, E) Result(T, E)
#define Result_(T, E) \
struct { \
    union { bool ok; size_t _ok; } is; \
    union { \
        T val; \
        E err; \
    } ret; \
} \
/**/

#define Result_ok(it) { .is.ok = true, .ret.val = (it), }
#define Result_err(it) { .is.ok = false, .ret.err = (it), }
