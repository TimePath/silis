#pragma once
// SPDX-License-Identifier: AFL-3.0

#define IF_COND(cond, t, f) IF_COND_1(cond, t, f)
#define IF_COND_1(cond, t, f) IF_COND_DISPATCH_##cond(t, f)
#define IF_COND_DISPATCH_0(t, f) f
#define IF_COND_DISPATCH_1(t, f) t

#if defined(TEST_MACROS)
// expect IF_COND(1, true, false) : true
IF_COND(1, true, false)
// expect IF_COND(0, true, false) : false
IF_COND(0, true, false)
#endif

#define IS_EMPTY(...) IS_EMPTY_1(__VA_OPT__(,))
#define IS_EMPTY_1(...) IS_EMPTY_2(__VA_ARGS__, 0, 1)
#define IS_EMPTY_2(_0, _1, ret, ...) ret

#if defined(TEST_MACROS)
// expect IS_EMPTY() : 1
IS_EMPTY()
// expect IS_EMPTY(1) : 0
IS_EMPTY(1)
// expect IS_EMPTY(1, 2) : 0
IS_EMPTY(1, 2)
#endif

#define IF_EMPTY(list, t, f) IF_COND(IS_EMPTY list, t, f)

#if defined(TEST_MACROS)
// expect IF_EMPTY((), empty, nonempty) : empty
IF_EMPTY((), empty, nonempty)
// expect IF_EMPTY((), empty, nonempty) : nonempty
IF_EMPTY((1), empty, nonempty)
// expect IF_EMPTY((), empty, nonempty) : nonempty
IF_EMPTY((1, 2), empty, nonempty)
#endif

#define LIST_SIZE(...) IF_EMPTY((__VA_ARGS__), 0, LIST_SIZE_1(__VA_ARGS__, 4, 3, 2, 1))
#define LIST_SIZE_1(_4, _3, _2, _1, ret, ...) ret

#if defined(TEST_MACROS)
// expect LIST_SIZE : 0
LIST_SIZE()
// expect LIST_SIZE : 1
LIST_SIZE(1)
// expect LIST_SIZE : 2
LIST_SIZE(2, 2)
#endif

#define LIST_GET(list, idx) LIST_GET_##idx list
#define LIST_GET_0(ret, ...) ret
#define LIST_GET_1(_0, ret, ...) ret
#define LIST_GET_2(_0, _1, ret, ...) ret
#define LIST_GET_3(_0, _1, _2, ret, ...) ret

#if defined(TEST_MACROS)
// expect LIST_GET((1, 2, 3), 0) : 1
LIST_GET((1, 2, 3), 0)
// expect LIST_GET((1, 2, 3), 1) : 2
LIST_GET((1, 2, 3), 1)
// expect LIST_GET((1, 2, 3), 2) : 3
LIST_GET((1, 2, 3), 2)
#endif

#define LIST_HEAD(list) (LIST_GET(list, 0))

#if defined(TEST_MACROS)
// expect LIST_HEAD((1, 2, 3)) : (1)
LIST_HEAD((1, 2, 3))
// expect LIST_HEAD((1, 2)) : (1)
LIST_HEAD((1, 2))
// expect LIST_HEAD((1)) : (1)
LIST_HEAD((1))
// expect LIST_HEAD(()) : ()
LIST_HEAD(())
#endif

#define LIST_TAIL(list) LIST_TAIL_1 list
#define LIST_TAIL_1(head, ...) (__VA_ARGS__)

#if defined(TEST_MACROS)
// expect LIST_TAIL((1, 2, 3)) : (2, 3)
LIST_TAIL((1, 2, 3))
// expect LIST_TAIL((1, 2)) : (2)
LIST_TAIL((1, 2))
// expect LIST_TAIL((1)) : ()
LIST_TAIL((1))
// expect LIST_TAIL(()) : ()
LIST_TAIL(())
#endif

#define LIST_SPREAD(list) LIST_SPREAD_1 list
#define LIST_SPREAD_1(...) __VA_ARGS__

#if defined(TEST_MACROS)
// expect LIST_SPREAD((1, 2, 3)) : 1, 2, 3
LIST_SPREAD((1, 2, 3))
#endif

#define MAP(transform, delimiter, list) MAP_1(transform, delimiter, LIST_SPREAD(list))
#define PARENS ()
#define EXPAND(...) EXPAND_1(EXPAND_1(EXPAND_1(EXPAND_1(__VA_ARGS__))))
#define EXPAND_1(...) EXPAND_2(EXPAND_2(EXPAND_2(EXPAND_2(__VA_ARGS__))))
#define EXPAND_2(...) EXPAND_3(EXPAND_3(EXPAND_3(EXPAND_3(__VA_ARGS__))))
#define EXPAND_3(...) EXPAND_4(EXPAND_4(EXPAND_4(EXPAND_4(__VA_ARGS__))))
#define EXPAND_4(...) __VA_ARGS__
#define MAP_1(transform, delimiter, ...) \
    __VA_OPT__(EXPAND(MAP_2(transform, delimiter, __VA_ARGS__)))
#define MAP_2_ADDR() MAP_2
#define MAP_2(transform, delimiter, it, ...) \
    transform(it) \
    __VA_OPT__(delimiter()) \
    __VA_OPT__(MAP_2_ADDR PARENS (transform, delimiter, __VA_ARGS__))

#if defined(TEST_MACROS)
// expect MAP() : (1 * 2) + (2 * 2) + (3 * 2)
#define DOUBLE(it, ...) (it * 2)
#define DELIMITER_PLUS() +
MAP(DOUBLE, DELIMITER_PLUS, (1, 2, 3))
#endif
