#pragma once

#if TARGET_COMPILER_MSVC
#define __extension__
#define INLINE inline
#else
#define INLINE __attribute__((always_inline)) inline
#endif

#define PADDING(n) uint8_t _padding[n];
#define BIT_PADDING(T, N) __extension__ T _padding : N;

#define MACRO_BEGIN if (1) {
#define MACRO_END } else ((void) 0)

#define ARRAY_LEN(x) ((sizeof(x) / sizeof((x)[0])))

#define OFFSETOF(T, fld) ((size_t) &(((T *) 0)->fld))

#if TARGET_COMPILER_MSVC
#pragma warning(disable:4116)
#endif
#define CAST(T, U, it) ((union { U from; T to; }) { .from = (it) }.to)

#define EMPTY()
#define NOEXPAND(...) __VA_ARGS__ EMPTY()

#define CAT2(_0, _1) _CAT2(_0, _1)
#define _CAT2(_0, _1) _0 ## _1
#define CAT3(_0, _1, _2) _0 ## _1 ## _2
#define CAT4(_0, _1, _2, _3) _0 ## _1 ## _2 ## _3

#define STRINGIFY(self) #self

#if TARGET_COMPILER_CLANG
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#if TARGET_COMPILER_CLANG
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

#if TARGET_COMPILER_TCC
#define ENUM_PADDING PADDING(20)
#else
#define ENUM_PADDING
#endif

#define ENUM(id) \
typedef struct { \
    union { \
        enum { \
            CAT2(id, _INVALID), \
            id(id, _ENUM_TAG) \
        } val; \
        size_t _val; \
        ENUM_PADDING \
    } kind; \
    union { id(id, _ENUM_VARIANT) } u; \
} id; \
/**/

#define _ENUM_TAG(id, name, def) CAT3(id, _, name),

#define _ENUM_VARIANT(id, name, def) struct def name;

#define _ENUM_VAL(id, variant) .kind.val = CAT3(id, _, variant), .u.variant
#define ENUM_VAL(id, variant, it) (id) { _ENUM_VAL(id, variant) = it }

#define Ref(T) CAT2(Ref__, T)
#define Ref_instantiate(T, U) typedef Ref_(T, size_t) Ref(T)
#define Ref_(T, U) \
struct { \
    struct { \
        void *(*deref)(void *owner); \
        void *owner; \
        U id; \
    } priv; \
}

#define Ref_value(self) ((self).priv.id)

#define Ref_eq(a, b) (Ref_value(a) == Ref_value(b))

#define Ref_null {{ NULL, NULL, 0 }}
#define Ref_max {{ NULL, NULL, (size_t) -1 }}

#define Ref_fromIndexCheck(T, i) ((i) < ((size_t) Ref_value((Ref(T)) Ref_max)))
#define Ref_fromIndex(idx) {{ NULL, NULL, (idx) + 1 }}

#define Ref_toBool(self) (Ref_value(self) != 0)
#define Ref_toIndex(self) (Ref_value(self) - 1)
