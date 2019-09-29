#pragma once

#if TARGET_COMPILER_MSVC
#define __extension__
#define INLINE inline
#else
#define INLINE __attribute__((always_inline)) inline
#endif

#define PADDING(n) uint8_t _padding[n]
#define BIT_PADDING(T, N) __extension__ T _padding : N

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
#define ADT_PADDING PADDING(20);
#else
#define ADT_PADDING
#endif

#define ADT(T) CAT2(ADT__, T)
// T: (_: CTX, case: (_: CTX, name: symbol, def: {}) => ...) => ...
#define ADT_instantiate(T) \
    typedef struct { ADT_instantiate_(T) } ADT(T); \
    typedef ADT(T) T
#define ADT_instantiate_(T) ADT_(T, T, ADT_TAG_GEN, T, ())
#define ADT_(macro, id, tag_func, tag_data, extra) ADT__(macro, (id, tag_func, tag_data, extra))
#define ADT__(macro, _) \
union { \
    ADT_TAG(_ADT_CTX_TAG_FUNC _, _, _ADT_CTX_TAG_DATA _) val; \
    size_t _val; \
    ADT_PADDING \
} kind; \
union { macro(_, _ADT_VARIANT) } u; \
/**/
#define ADT_TAG(f, _, d) f(_, d)

#define ADT_TAG_USE(_, data) data
#define ADT_TAG_GEN(_, macro) \
enum { \
    _ADT_TAG(_, INVALID, {}) \
    macro(_, _ADT_TAG) \
}

#define _ADT_CTX_ID(id, tag_func, tag_data, extra) id
#define _ADT_CTX_TAG_FUNC(id, tag_func, tag_data, extra) tag_func
#define _ADT_CTX_TAG_DATA(id, tag_func, tag_data, extra) tag_data
#define _ADT_CTX_EXTRA(id, tag_func, tag_data, extra) extra

#define _ADT_TAG(_, name, def) _ADT_TAG_(_ADT_CTX_ID _, name),
#define _ADT_TAG_(adt, id) CAT3(adt, _, id)

#define _ADT_VARIANT(_, name, def) def name;

#define _ADT_VAL(macro, variant) .kind.val = CAT3(macro, _, variant), .u.variant
#define ADT_VAL(macro, variant, it) (macro) { _ADT_VAL(macro, variant) = it }

#define Ref(T) CAT2(Ref__, T)
#define Ref_instantiate(T, U) typedef Ref_(T, size_t) Ref(T)
#define Ref_(T, U) \
struct { \
    struct { \
        void const *container; \
        void *(*deref)(void const *container, U id); \
        U id; \
    } priv; \
}

#define Ref_value(self) ((self).priv.id)

#define Ref_eq(a, b) (Ref_value(a) == Ref_value(b))

#define Ref_null {{ NULL, NULL, 0 }}
#define Ref_max {{ NULL, NULL, (size_t) -1 }}

#define Ref_from(container, deref, idx) {{ (container), (deref), (idx) + 1 }}

#define Ref_fromIndexCheck(T, i) ((i) < ((size_t) Ref_value((Ref(T)) Ref_max)))
#define Ref_fromIndex(idx) {{ NULL, NULL, (idx) + 1 }}

#define Ref_toBool(self) (Ref_value(self) != 0)
#define Ref_toIndex(self) (Ref_value(self) - 1)
