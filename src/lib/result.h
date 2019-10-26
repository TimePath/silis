#pragma once

#include "macro.h"

typedef enum {
    Result_INVALID,
    Result_Ok,
    Result_Err,
} Result_e;

#define Result(T, E) CAT4(Result__, T, __, E)
#define Result_instantiate(T, E) typedef Result_(T, E) Result(T, E)
#define Result_(T, E) struct { ADT_(Result_ADT, Result, ADT_TAG_USE, Result_e, (T, E)) }

#define Result_ADT(_, case) \
    case(_, Ok, Result_ADT_T(_)) \
    case(_, Err, Result_ADT_E(_)) \
    /**/

#define Result_ADT_Eval(it) it

#define Result_ADT_T(_) Result_ADT_Eval(Result_ADT_T_ _ADT_CTX_EXTRA _)
#define Result_ADT_T_(T, E) T

#define Result_ADT_E(_) Result_ADT_Eval(Result_ADT_E_ _ADT_CTX_EXTRA _)
#define Result_ADT_E_(T, E) E

#define Result_ok(it) { .kind.val = Result_Ok, .u.Ok = (it), }
#define Result_err(it) { .kind.val = Result_Err, .u.Err = (it), }
