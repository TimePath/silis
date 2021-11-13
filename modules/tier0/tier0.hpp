#pragma once

#define func auto
#define let const auto &
#define var auto

#define implicit

#if 0
#elif defined(__clang__)
#define COMPILER_IS_CLANG 1
#define COMPILER_IS_GCC 0
#elif defined(__GNUG__)
#define COMPILER_IS_CLANG 0
#define COMPILER_IS_GCC 1
#else
#error "Unknown compiler"
#endif

#if COMPILER_IS_CLANG
#define ATTR_TYPESTATE_TYPE [[clang::consumable(unknown)]]
#define ATTR_TYPESTATE_CTOR(state) [[clang::return_typestate(state)]]
#define ATTR_TYPESTATE_PRECONDITION(state) [[clang::callable_when(state)]]
#define ATTR_TYPESTATE_ASSERTS(state) [[clang::test_typestate(state)]]
#else
#define ATTR_TYPESTATE_TYPE
#define ATTR_TYPESTATE_CTOR(state)
#define ATTR_TYPESTATE_PRECONDITION(state)
#define ATTR_TYPESTATE_ASSERTS(state)
#endif

namespace tier0 {}

// meta
namespace tier0 {

#include "../macros.h"

#define METAFUNC_TYPE(name, args) \
    template <MAP(METAFUNC_IMPL_DECLARE_ARGS, METAFUNC_IMPL_DELIMITER_COMMA, args)> \
    struct name##_s; \
    template <MAP(METAFUNC_IMPL_DECLARE_ARGS, METAFUNC_IMPL_DELIMITER_COMMA, args)> \
    using name = typename name##_s<MAP(METAFUNC_IMPL_DECLARE_FORWARD, METAFUNC_IMPL_DELIMITER_COMMA, args)>::type;
#define METAFUNC_TYPE_IMPL(name, spec, args, val) \
    template <MAP(METAFUNC_IMPL_DECLARE_ARGS, METAFUNC_IMPL_DELIMITER_COMMA, args)> \
    struct name##_s IF_EMPTY(spec, METAFUNC_IMPL_SPECIALIZE_0, METAFUNC_IMPL_SPECIALIZE_1)(spec) { \
        using type = val; \
    };

#define METAFUNC_VALUE(name, args) \
    template <MAP(METAFUNC_IMPL_DECLARE_ARGS, METAFUNC_IMPL_DELIMITER_COMMA, args)> \
    struct name##_s; \
    template <MAP(METAFUNC_IMPL_DECLARE_ARGS, METAFUNC_IMPL_DELIMITER_COMMA, args)> \
    concept name = name##_s<MAP(METAFUNC_IMPL_DECLARE_FORWARD, METAFUNC_IMPL_DELIMITER_COMMA, args)>::value;
#define METAFUNC_VALUE_IMPL(name, spec, args, val) \
    template <MAP(METAFUNC_IMPL_DECLARE_ARGS, METAFUNC_IMPL_DELIMITER_COMMA, args)> \
    struct name##_s IF_EMPTY(spec, METAFUNC_IMPL_SPECIALIZE_0, METAFUNC_IMPL_SPECIALIZE_1)(spec) { \
        static constexpr auto value = val; \
    };

#define METAFUNC_IMPL_DELIMITER_COMMA() ,
#define METAFUNC_IMPL_DECLARE_ARGS(it) LIST_GET(it, 1) LIST_GET(it, 0)
#define METAFUNC_IMPL_DECLARE_FORWARD(it) LIST_GET(it, 0)
#define METAFUNC_IMPL_SPECIALIZE_0(spec)
#define METAFUNC_IMPL_SPECIALIZE_1(spec) <MAP(METAFUNC_IMPL_SPECIALIZE_1_ARG, METAFUNC_IMPL_DELIMITER_COMMA, spec)>
#define METAFUNC_IMPL_SPECIALIZE_1_ARG(it) it

#if defined(TEST_MACROS)
    // expect METAFUNC_TYPE : template <bool predicate , typename T , typename F> struct cond_s; template <bool predicate , typename T , typename F> using cond = typename cond_s<predicate , T , F>::type;
    METAFUNC_TYPE(cond, ((predicate, bool), (T, typename), (F, typename)))
    // expect METAFUNC_TYPE_IMPL : template <typename T , typename F> struct cond_s <true , T , F> { using type = T; };
    METAFUNC_TYPE_IMPL(cond, (true, T, F), ((T, typename), (F, typename)), T)
    // expect METAFUNC_TYPE_IMPL : template <typename T , typename F> struct cond_s <false , T , F> { using type = F; };
    METAFUNC_TYPE_IMPL(cond, (false, T, F), ((T, typename), (F, typename)), F)
#endif

    METAFUNC_TYPE(remove_reference, ((T, typename)))
    METAFUNC_TYPE_IMPL(remove_reference, (T && ), ((T, typename)), T)
    METAFUNC_TYPE_IMPL(remove_reference, (T & ), ((T, typename)), T)
    METAFUNC_TYPE_IMPL(remove_reference, (), ((T, typename)), T)

    METAFUNC_TYPE(cond, ((predicate, bool), (T, typename), (F, typename)))
    METAFUNC_TYPE_IMPL(cond, (true, T, F), ((T, typename), (F, typename)), T)
    METAFUNC_TYPE_IMPL(cond, (false, T, F), ((T, typename), (F, typename)), F)

    METAFUNC_VALUE(is_same, ((T, typename), (U, typename)))
    METAFUNC_VALUE_IMPL(is_same, (T, T), ((T, typename)), true)
    METAFUNC_VALUE_IMPL(is_same, (), ((T, typename), (U, typename)), false)

    METAFUNC_VALUE(is_const, ((T, typename)))
    METAFUNC_VALUE_IMPL(is_const, (const T), ((T, typename)), true)
    METAFUNC_VALUE_IMPL(is_const, (), ((T, typename)), false)

    METAFUNC_TYPE(add_const, ((T, typename)))
    METAFUNC_TYPE_IMPL(add_const, (const T), ((T, typename)), const T)
    METAFUNC_TYPE_IMPL(add_const, (), ((T, typename)), const T)

    METAFUNC_TYPE(remove_const, ((T, typename)))
    METAFUNC_TYPE_IMPL(remove_const, (const T), ((T, typename)), T)
    METAFUNC_TYPE_IMPL(remove_const, (), ((T, typename)), T)

    namespace detail {
        using _false = char[1];
        using _true = char[2];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-template"

        template<typename T>
        static _true *test(T *) { return nullptr; }

        template<typename>
        static _false *test(void *) { return nullptr; }

#pragma clang diagnostic pop
    }

    METAFUNC_VALUE(is_base_of, ((Super, typename), (Self, typename)))
    METAFUNC_VALUE_IMPL(is_base_of, (), ((Super, typename), (Self, typename)),
                        sizeof(*detail::test<Super>(static_cast<Self *>(nullptr))) == sizeof(detail::_true))
}

// memory model
namespace tier0 {
    using cstring = char const *;

    template<typename T>
    using ref = const T &;

    template<typename T>
    using mut_ref = T &;

    template<typename T>
    using movable = T &&;

    template<typename T>
    constexpr movable<remove_reference<T>> move(movable<T> value) {
        return movable<remove_reference<T>>(value);
    }

    template<typename T, typename U>
    constexpr T exchange(mut_ref<T> target, movable<U> replacement) {
        var ret = move(target);
        target = replacement;
        return ret;
    }

    template<typename T>
    constexpr mut_ref<T> operator_move_assign(mut_ref<T> self, movable<T> other) {
        if (&self != &other) {
            self.~T();
            new(&self) T(move(other));
        }
        return self;
    }

    template<typename T>
    struct native_s;

    template<typename T>
    using Native = typename native_s<T>::type;
}

// order
namespace tier0 {
    enum class Order {
        Before = -1,
        Undefined,
        After,
    };
}

// primitives
namespace tier0 {
    namespace detail {
        METAFUNC_VALUE(is_arithmetic, ((T, typename)))

        template<typename T>
        struct is_arithmetic_s {
            static constexpr auto value = is_same < bool, T>
            || is_same<char, T> || is_same<unsigned char, T> || is_same<signed char, T>
            || is_same<short int, T> || is_same<short unsigned int, T> || is_same<short signed int, T>
            || is_same<int, T> || is_same<unsigned int, T> || is_same<signed int, T>
            || is_same<long int, T> || is_same<long unsigned int, T> || is_same<long signed int, T>
            || is_same<float, T>
            || is_same<double, T>;
        };

        template<typename T>
        struct WordTraits;

        struct WordTag {
        };

        template<typename T> requires is_arithmetic<T>
        struct Word : WordTag {
            friend WordTraits<Word>;
            T data_;

            explicit constexpr Word() : Word(0) {}

            /** Copy */
            implicit constexpr Word(ref<Word> other) : data_(other.data_) {}

            /** Exact primitive match */
            template<typename T2>
            requires is_same<T, T2>
            implicit constexpr Word(T2 value) : data_(value) {}

            /** Inexact primitive match */
            template<typename U>
            requires (!is_same < T, U > and is_arithmetic < U >)
            explicit constexpr Word(U value) : data_(T(value)) {}

            /** Cast from */
            template<typename Other>
            requires (WordTraits<Other>::isWord && !is_same < Other, Word >)
            explicit Word(Other word) : Word(Native<Other>(word)) {}

            /** Cast to */
            implicit constexpr operator T() const { return data_; }

            // operators

            constexpr bool operator==(ref<Word> other) const { return (*this).data_ == other.data_; }

            constexpr Order compareTo(ref<Word> other) const {
                let a = this->data_;
                let b = other.data_;
                if (a < b) {
                    return Order::Before;
                }
                if (a > b) {
                    return Order::After;
                }
                return Order::Undefined;
            }
        };

        template<typename T> requires is_base_of<WordTag, T>
        struct WordTraits<T> {
            static const bool isWord = true;
            using underlying = decltype(static_cast<T *>(nullptr)->data_);
        };
    }

    template<typename T> requires detail::WordTraits<T>::isWord
    struct native_s<T> {
        using type = typename detail::WordTraits<T>::underlying;
    };

    struct Unit {
        explicit Unit() = default;
    };

    struct Boolean : detail::Word<bool> {
        using Word::Word;
    };
    struct Char : detail::Word<char> {
        using Word::Word;
    };
    struct Byte : detail::Word<unsigned char> {
        using Word::Word;
    };
    struct Int8 : detail::Word<signed char> {
        using Word::Word;
    };
    struct Short : detail::Word<short signed int> {
        using Word::Word;
    };
    struct UShort : detail::Word<short unsigned int> {
        using Word::Word;
    };
    struct Int : detail::Word<signed int> {
        using Word::Word;
    };
    struct UInt : detail::Word<unsigned int> {
        using Word::Word;
    };
    struct Long : detail::Word<long signed int> {
        using Word::Word;
    };
    struct ULong : detail::Word<long unsigned int> {
        using Word::Word;
    };
    struct Float : detail::Word<float> {
        using Word::Word;
    };
    struct Double : detail::Word<double> {
        using Word::Word;
    };
    using Size = ULong;

#pragma GCC poison signed
#pragma GCC poison unsigned
#pragma GCC poison bool
#pragma GCC poison char
#pragma GCC poison short
#pragma GCC poison int
#pragma GCC poison long
#pragma GCC poison float
#pragma GCC poison double
}

// assert
namespace tier0 {
    [[noreturn]] inline void die() {
        throw 0;
    }

#define assert(flag) do { if (!(flag)) die(); } while (0)
}

// pointer
namespace tier0 {
    template<typename T>
    using _ptr = T *;

    template<typename T, typename R>
    struct mptr;

    template<typename T, typename R>
    mptr(R T::*) -> mptr<T, R>;

    template<typename T, typename R>
    struct mptr {
        using native = R T::*;
        native data_;

        implicit constexpr mptr(native data) : data_(data) {}

        constexpr ref<R> get(ref<T> obj) const { return obj.*data_; }

        constexpr mut_ref<R> get(mut_ref<T> obj) const { return obj.*data_; }

        template<typename... Ts>
        constexpr auto call(mut_ref<T> obj, Ts... args) const { return (obj.*data_)(args...); }
    };

    template<typename T>
    struct ptr;

    template<typename Self, Boolean isVoid>
    struct ptr_mixin_s;

    template<typename Self>
    struct ptr_mixin;

    template<typename T>
    struct ptr_mixin<ptr<T>> {
        using type = ptr_mixin_s<ptr<T>, is_same < void, remove_const < T>>>;
    };

    template<typename T>
    struct ptr : public ptr_mixin<ptr<T>>::type {
        using base = typename ptr_mixin<ptr<T>>::type;
        using native = _ptr<T>;
        native data_;

        using base::base;

        constexpr ptr() = delete;

        template<typename U>
        static constexpr ptr reinterpret(_ptr<U> value) { return reinterpret_cast<native>(value); }

        implicit constexpr ptr(native data) : data_(data) { check(); }

        constexpr ref<native> value() const { return check(), data_; }

        constexpr mut_ref<native> value() { return check(), data_; }

        implicit constexpr operator native() const { return value(); }

        explicit constexpr operator Native<Boolean>() { return value(); }

        constexpr native operator->() const { return value(); }

    private:
        constexpr void check() const { assert(!!data_); }
    };

    template<typename T>
    Order operator<=>(ref<ptr<T>> a, ref<ptr<T>> b) {
        return Size(Native<Size>(a.data_)).compareTo(Size(Native<Size>(b.data_)));
    }

    template<typename T>
    struct native_s<ptr<T>> {
        using type = _ptr<T>;
    };

    template<typename Self>
    struct ptr_mixin_s<Self, true> {
    };

    template<typename Self>
    struct ptr_mixin_s<Self, false> {
    private:
        template<typename>
        struct extract_t;
        template<typename T>
        struct extract_t<ptr<T>> {
            using type = T;
        };
        using T = typename extract_t<Self>::type;

        constexpr Self const *super() const { return static_cast<Self const *>(this); }

        constexpr Self *super() { return static_cast<Self *>(this); }

    public:
        constexpr ptr_mixin_s() = default;

        explicit constexpr ptr_mixin_s(ptr<void> p) { super()->data_ = Self::reinterpret(p.operator->()); }

        implicit constexpr operator ptr<void>() const { return ptr<void>::reinterpret(super()->value()); }

        constexpr ref<T> operator[](Int index) const { return super()->value()[index]; }

        constexpr ref<T> operator*() const { return *super()->value(); }

        constexpr mut_ref<T> operator[](Int index) { return super()->value()[index]; }

        constexpr mut_ref<T> operator*() { return *super()->value(); }
    };
}

// placement new

constexpr tier0::Native<tier0::ptr<void>>
operator new(tier0::Native<tier0::Size> count, tier0::ptr<void> place) noexcept;

constexpr tier0::Native<tier0::ptr<void>>
operator new[](tier0::Native<tier0::Size> count, tier0::ptr<void> place) noexcept;

void operator delete(tier0::Native<tier0::ptr<void>> ptr, tier0::ptr<void> place) noexcept;

void operator delete[](tier0::Native<tier0::ptr<void>> ptr, tier0::ptr<void> place) noexcept;

#define IMPLEMENTS_PLACEMENT 1

#if IMPLEMENTS_PLACEMENT

constexpr inline tier0::Native<tier0::ptr<void>> operator new(tier0::Native<tier0::Size> count, tier0::ptr<void> place) noexcept {
    (void) count;
    return place;
}

constexpr inline tier0::Native<tier0::ptr<void>> operator new[](tier0::Native<tier0::Size> count, tier0::ptr<void> place) noexcept {
    (void) count;
    return place;
}

inline void operator delete(tier0::Native<tier0::ptr<void>> ptr, tier0::ptr<void> place) noexcept {
    (void) ptr;
    (void) place;
}

inline void operator delete[](tier0::Native<tier0::ptr<void>> ptr, tier0::ptr<void> place) noexcept {
    (void) ptr;
    (void) place;
}

#endif

// https://en.cppreference.com/w/cpp/named_req
namespace tier0 {
    struct DisableDefaultConstructible {
    private:
        DisableDefaultConstructible() = delete;

    protected:
        DisableDefaultConstructible(Unit) {}
    };

    struct DisableDestructible {
    protected:
        ~DisableDestructible() = delete;
    };

    struct DisableCopyConstructible {
    private:
        DisableCopyConstructible(ref<DisableCopyConstructible>) = delete;

    protected:
        DisableCopyConstructible(Unit) {}
    };

    struct DisableCopyAssignable {
    private:
        mut_ref<DisableCopyAssignable> operator=(ref<DisableCopyAssignable>) = delete;
    };

    struct DisableMoveConstructible {
    private:
        DisableMoveConstructible(movable<DisableMoveConstructible>) = delete;

    protected:
        DisableMoveConstructible(Unit) {}
    };

    struct DisableMoveAssignable {
    private:
        mut_ref<DisableMoveAssignable> operator=(movable<DisableMoveAssignable>) = delete;
    };
}

// meta
namespace tier0 {
    template<typename... Ts>
    struct Structs;

    template<typename... Ts>
    struct Structs : Ts ... {
    };

    template<typename... Ts>
    struct TypeList;

    template<typename T, T... values>
    struct ValueList;

#if !__has_builtin(__make_integer_seq)
    namespace detail {
        template<Native<Size> n>
        struct make_seq_s;

        template<Native<Size> n>
        using make_seq = typename make_seq_s<n>::type;

        template<Native<Size> n>
        struct make_seq_lo_s;

        template<Native<Size> n>
        using make_seq_lo = make_seq_lo_s<n>;

        template<typename T, Native<Size>... tail>
        struct make_seq_recurse_s;

        template<Native<Size> n, Native<Size>... values>
        using make_seq_recurse = typename make_seq_recurse_s<make_seq<n>, values...>::type;

        constexpr Native<Size> seq_bits = 3;

        consteval Native<Size> seq_lo(Native<Size> n) { return n & ((1 << seq_bits) - 1); }

        consteval Native<Size> seq_hi(Native<Size> n) { return n >> seq_bits; }

        template<Native<Size> n>
        using make_seq_generic = typename make_seq_lo<seq_lo(n)>::template hi<n>;

        template<Native<Size> n>
        struct make_seq_s {
            using type = make_seq_generic<n>;
        };

        template<>
        struct make_seq_s<0> {
            using type = ValueList<Native<Size>>;
        };
        template<>
        struct make_seq_s<1> {
            using type = ValueList<Native<Size>, 0>;
        };
        template<>
        struct make_seq_s<2> {
            using type = ValueList<Native<Size>, 0, 1>;
        };
        template<>
        struct make_seq_s<3> {
            using type = ValueList<Native<Size>, 0, 1, 2>;
        };
        template<>
        struct make_seq_s<4> {
            using type = ValueList<Native<Size>, 0, 1, 2, 3>;
        };
        template<>
        struct make_seq_s<5> {
            using type = ValueList<Native<Size>, 0, 1, 2, 3, 4>;
        };
        template<>
        struct make_seq_s<6> {
            using type = ValueList<Native<Size>, 0, 1, 2, 3, 4, 5>;
        };
        template<>
        struct make_seq_s<7> {
            using type = ValueList<Native<Size>, 0, 1, 2, 3, 4, 5, 6>;
        };

        template<>
        struct make_seq_lo_s<0> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n)>;
        };
        template<>
        struct make_seq_lo_s<1> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n), n - 1>;
        };
        template<>
        struct make_seq_lo_s<2> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n), n - 2, n - 1>;
        };
        template<>
        struct make_seq_lo_s<3> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n), n - 3, n - 2, n - 1>;
        };
        template<>
        struct make_seq_lo_s<4> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n), n - 4, n - 3, n - 2, n - 1>;
        };
        template<>
        struct make_seq_lo_s<5> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n), n - 5, n - 4, n - 3, n - 2, n - 1>;
        };
        template<>
        struct make_seq_lo_s<6> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n), n - 6, n - 5, n - 4, n - 3, n - 2, n - 1>;
        };
        template<>
        struct make_seq_lo_s<7> {
            template<Native<Size> n>
            using hi = make_seq_recurse<seq_hi(n), n - 7, n - 6, n - 5, n - 4, n - 3, n - 2, n - 1>;
        };

        template<Native<Size>... values, Native<Size>... tail>
        struct make_seq_recurse_s<ValueList<Native<Size>, values...>, tail...> {
            static const auto n = sizeof...(values);
            using type = ValueList<Native<Size>,
                    ((0 * n) + values)...,
                    ((1 * n) + values)...,
                    ((2 * n) + values)...,
                    ((3 * n) + values)...,
                    ((4 * n) + values)...,
                    ((5 * n) + values)...,
                    ((6 * n) + values)...,
                    ((7 * n) + values)...,
                    tail...
            >;
        };
    }

    template<template<typename X, X...> typename C, typename T, T n>
    using __make_integer_seq = typename detail::make_seq<n>::template transform<C>;
#endif

#if !__has_builtin(__type_pack_element)
#define TYPE_PACK_ELEMENT_FALLBACK 2
#if TYPE_PACK_ELEMENT_FALLBACK == 1
    namespace detail {
        // METAFUNC_TYPE(get, ((I, Size), (Ts, typename...)))

        template<Native<Size> I, typename... Ts>
        struct get_s;

        template<Native<Size> I, typename... Ts>
        using get = typename get_s<I, Ts...>::type;

        METAFUNC_TYPE_IMPL(get, (0, T, Ts...), ((T, typename),(Ts, typename...)), T)
        METAFUNC_TYPE_IMPL(get, (I, T, Ts...), ((I, Native<Size>), (T, typename),(Ts, typename...)),
                           get<I - 1 METAFUNC_IMPL_DELIMITER_COMMA() Ts...>)
    }

    template<Native<Size> I, typename... Ts>
    using __type_pack_element = detail::get<I, Ts...>;
#endif
#if TYPE_PACK_ELEMENT_FALLBACK == 2
    namespace detail {
        template<Native<Size>, typename T>
        struct IndexedType {
            using type = T;
        };

        template<typename Ts, typename Is>
        struct IndexedTypes;

        template<typename... Ts, Native<Size>... Is>
        struct IndexedTypes<TypeList<Ts...>, ValueList<Native<Size>, Is...>> : IndexedType<Is, Ts> ... {
        };

        template<Native<Size> I, typename T>
        IndexedType<I, T> get_base(ref<IndexedType<I, T>>);
    }

    template<Native<Size> I, typename... Ts>
    using __type_pack_element = typename decltype(
    detail::get_base<I>(
            detail::IndexedTypes<TypeList<Ts...>, __make_integer_seq<ValueList, Native<Size>, sizeof...(Ts)>>{}
    ))::type;
#endif
#endif

    template<typename... Ts>
    struct TypeList {
        template<Native<Size> I>
        using get = __type_pack_element<I, Ts...>;

        METAFUNC_TYPE(concat, ((U, typename)))
        METAFUNC_TYPE_IMPL(concat, (TypeList<Us...>), ((Us, typename...)),
                           TypeList<Ts... METAFUNC_IMPL_DELIMITER_COMMA() Us...>)

        template<template<typename> typename F>
        using map = TypeList<F<Ts>...>;

        template<typename T, template<typename> typename F>
        using mapv = ValueList<T, F<Ts>::value...>;
    };

    template<typename T, T... values>
    struct ValueList {
        template<template<typename X, X...> typename C>
        using transform = C<T, values...>;
    };

    METAFUNC_TYPE(collect2, ((hasNext, Boolean), (T, typename)))
    template<typename Node> using collect = collect2<Node::hasNext, Node>;
    METAFUNC_TYPE_IMPL(collect2, (true, T), ((T, typename)),
                       typename TypeList<T>::template concat<collect<typename T::next>>)
    METAFUNC_TYPE_IMPL(collect2, (false, T), ((T, typename)), TypeList<T>)

#define apply_func template<typename...> typename
    METAFUNC_TYPE(apply, ((F, apply_func), (T, typename)))
    METAFUNC_TYPE_IMPL(apply, (F, TypeList<Ts...>), ((F, apply_func),(Ts, typename...)), F < Ts...>)
#undef apply_func

#define applyv_func template<T...> typename
    METAFUNC_TYPE(applyv, ((T, typename), (F, applyv_func), (X, typename)))
    METAFUNC_TYPE_IMPL(applyv, (T, F, ValueList<T, values...>), ((T, typename), (F, applyv_func),(values, T...)),
                       F < values...>)
#undef applyv_func

    template<Native<Size> i = 0>
    struct CounterIterator {
        constexpr static auto hasNext = true;
        using next = CounterIterator<i + 1>;

        constexpr static auto value = i;
    };

    template<typename... Ts>
    struct PackIterator;

    template<typename T, typename... Rest>
    struct PackIterator<T, Rest...> {
        constexpr static auto hasNext = sizeof...(Rest) != 0;
        using next = PackIterator<Rest...>;

        using type = T;
    };

    template<typename First, typename Second>
    struct ZipIterator {
        constexpr static auto hasNext = First::hasNext && Second::hasNext;
        using next = ZipIterator<typename First::next, typename Second::next>;

        using first = First;
        using second = Second;
    };

#if __has_builtin(__make_integer_seq)
    template <typename... Ts>
    struct EnumerateContainer {
        using list = TypeList<Ts...>;

        template <Native<Size> i>
        struct EnumerateElement {
            struct first { static constexpr var value = i; };
            struct second { using type = typename list::template get<i>; };
        };

        template <typename T, T... Is>
        struct EnumerateList {
            using combined = TypeList<EnumerateElement<Is>...>;
        };

        using type = typename __make_integer_seq<EnumerateList, Native<Size>, sizeof...(Ts)>::combined;
    };

    template <typename... Ts>
    using Enumerate = typename EnumerateContainer<Ts...>::type;
#else
    template<typename... Ts>
    using Enumerate = collect<ZipIterator<CounterIterator<>, PackIterator<Ts...>>>;
#endif

    template<Native<Size>... values>
    using NativeSizeValueList = ValueList<Native<Size>, values...>;

    template<typename T>
    struct ToIndex {
        static constexpr auto value = T::first::value;
    };

    template<typename... Ts>
    using Indices = applyv<Native<Size>, NativeSizeValueList, typename Enumerate<Ts...>::template mapv<Native<Size>, ToIndex>>;
}

// structured binding
namespace tier0 {
    namespace detail {
        template<typename T, T v>
        struct property;

        template<typename T, typename R, R T::*v>
        struct property<R T::*, v> {
            using type = R;

            static ref<type> get(ref<T> self) { return mptr(v).get(self); }

            static mut_ref<type> get(mut_ref<T> self) { return mptr(v).get(self); }
        };
    }

    template<typename... elements>
    struct tuple_elements {
        static constexpr Size size = sizeof...(elements);

        using types = TypeList<elements...>;

        template<Size I>
        using type = typename types::template get<I>::type;

        template<Size I, typename T>
        static ref<type<I>> get(ref<T> self) { return types::template get<I>::get(self); }

        template<Size I, typename T>
        static mut_ref<type<I>> get(mut_ref<T> self) { return types::template get<I>::get(self); }
    };

    template<typename... types>
    struct tuple_elements_builder {
        template<auto mptr>
        using add = tuple_elements_builder<types..., detail::property<decltype(mptr), mptr>>;

        using build = tuple_elements<types...>;
    };

    template<typename T>
    struct tuple_traits_s;
    template<typename T>
    using tuple_traits = tuple_traits_s<remove_reference<T>>;

    template<typename T>
    struct tuple_traits_s {
        using delegate = typename T::elements;

        static constexpr Size size = delegate::size;

        template<Size I>
        using type = typename delegate::template type<I>;

        template<Size I>
        static ref<type<I>> get(ref<T> self) { return delegate::template get<I, T>(self); }

        template<Size I>
        static mut_ref<type<I>> get(mut_ref<T> self) { return delegate::template get<I, T>(self); }
    };

    template<typename T>
    struct tuple_traits_s<const T> {
        using delegate = tuple_traits<remove_const < T>>;

        static constexpr Size size = delegate::size;

        template<Size I>
        using type = add_const<typename delegate::template type<I>>;

        template<Size I>
        static ref<type<I>> get(ref<T> self) { return delegate::template get<I>(self); }
    };

#define USING_STRUCTURED_BINDING \
    template<Native<Size> i, typename T> \
    constexpr ref<typename tuple_traits<T>::template type<i>> get(ref<T> self) { \
        return tuple_traits<T>::template get<i>(self); \
    } \
    template<Native<Size> i, typename T> \
    constexpr mut_ref<typename tuple_traits<T>::template type<i>> get(mut_ref<T> self) { \
        return tuple_traits<T>::template get<i>(self); \
    } \
    /**/

#define ENABLE_STRUCTURED_BINDING(T) \
    template<Native<Size> i> \
    constexpr ref<typename tuple_traits<T>::template type<i>> get() const { \
        return tuple_traits<T>::template get<i>(*this); \
    } \
    template<Native<Size> i> \
    constexpr mut_ref<typename tuple_traits<T>::template type<i>> get() { \
        return tuple_traits<T>::template get<i>(*this); \
    } \
    /**/
}

// structured binding
namespace std {
    template<typename T>
    struct tuple_size;

    template<::tier0::Native<::tier0::Size> i, typename T>
    struct tuple_element;
}

template<typename T>
struct std::tuple_size {
    static constexpr tier0::Native<tier0::Size> value = tier0::tuple_traits<T>::size;
};

template<tier0::Native<tier0::Size> i, typename T>
struct std::tuple_element {
    using type = typename tier0::tuple_traits<T>::template type<i>;
};

namespace tier0 {
    template<Size i = Size(0), typename T, Size n = tuple_traits<T>::size, typename F, typename A>
    constexpr A forEach(ref<T> tuple, F f, A acc) {
        if constexpr (i < n) {
            acc = f(acc, tuple.template get<i>(), i);
            return forEach<i + 1, T, n, F>(tuple, f, acc);
        }
        return acc;
    }

    template<Size i = Size(0), typename T, Size n = tuple_traits<T>::size, typename F, typename A>
    constexpr A forEach(ref<T> a, ref<T> b, F f, A acc) {
        if constexpr (i < n) {
            acc = f(acc, a.template get<i>(), b.template get<i>(), i);
            return forEach<i + 1, T, n, F>(a, b, f, acc);
        }
        return acc;
    }
}

// tuple
namespace tier0 {
    template<Size i, typename T>
    struct TupleLeaf {
        T value;
    };

    template<typename T>
    using ToTupleLeaf = TupleLeaf<T::first::value, typename T::second::type>;

    template<typename... Ts>
    using TupleStorage = apply<Structs, typename Enumerate<Ts...>::template map<ToTupleLeaf>>;

    template<typename... Ts>
    struct Tuple;

    template<typename... Ts>
    Tuple(Ts...) -> Tuple<Ts...>;

    template<typename... Ts>
    struct Tuple {
    private:
        TupleStorage<Ts...> data_;
    public:
        explicit Tuple(Ts... args) : data_{{args}...} {
        }

        struct [[maybe_unused]] elements {
            static constexpr Size size = sizeof...(Ts);

            template<Size I>
            using type = typename TypeList<Ts...>::template get<I>;

            template<Size I, typename T>
            static ref<type<I>> get(ref<T> self) { return self.data_.TupleLeaf<I, type<I>>::value; }

            template<Size I, typename T>
            static mut_ref<type<I>> get(mut_ref<T> self) { return self.data_.TupleLeaf<I, type<I>>::value; }
        };

        ENABLE_STRUCTURED_BINDING(Tuple)

        constexpr Boolean operator==(ref<Tuple> other) const { return equal(*this, other); }

    private:
        template<Native<Size>... Is>
        struct equality {
            static constexpr Boolean invoke(ref<Tuple> a, ref<Tuple> b) {
                return ((a.template get<Is>() == b.template get<Is>()) && ...);
            }
        };

        static constexpr Boolean equal(ref<Tuple> a, ref<Tuple> b) {
            return applyv<Native<Size>, equality, Indices<Ts...>>::invoke(a, b);
        }
    };

    template<typename... Ts>
    Order operator<=>(ref<Tuple<Ts...>> a, ref<Tuple<Ts...>> b) {
        auto compare = [&]<typename T>(Order acc, ref<T> aElem, ref<T> bElem, Size) -> Order {
            if (acc == Order::Undefined) {
                acc = aElem <=> bElem;
            }
            return acc;
        };
        return forEach(a, b, compare, Order::Undefined);
    }
}

// iterable

#define ENABLE_FOREACH_ITERABLE() \
    constexpr Native<ptr<void>> end() const { return nullptr; } \
    constexpr auto begin() const { return iterator(); } \
    constexpr auto begin() { return iterator(); } \
    /**/

#define ENABLE_FOREACH_ITERATOR(T) \
    constexpr Native<ptr<void>> end() const { return nullptr; } \
    constexpr ref<T> begin() const { return *this; } \
    constexpr mut_ref<T> begin() { return *this; } \
    /**/

// iterator

namespace tier0 {
    template<typename T>
    concept Iterator = requires(T it) {
        requires is_same<decltype(it.hasNext()), Boolean>;
        it.get();
        it.next();
    };
}

template<typename T>
requires tier0::Iterator<T>
inline constexpr tier0::Boolean
operator!=(tier0::ref<T> self, tier0::Native<tier0::ptr<void>>) { return self.hasNext(); }

template<typename T>
requires tier0::Iterator<T>
inline constexpr auto &operator*(tier0::ref<T> self) { return self.get(); }

template<typename T>
requires tier0::Iterator<T>
inline constexpr void operator++(tier0::mut_ref<T> self) { self.next(); }

// range
namespace tier0 {
    template<typename T>
    struct Range {
    private:
        T begin_;
        T end_;

        constexpr Range(T begin, T end) : begin_(begin), end_(end) {}

    public:
        static constexpr Range until(T begin, T end) { return {begin, end}; }

        constexpr Boolean contains(T i) { return (i >= begin_ and i < end_); }

        template<typename E>
        struct Iterator {
            Range self;

            constexpr Boolean hasNext() const { return self.begin_ < self.end_; }

            constexpr ref<E> get() const { return self.begin_; }

            constexpr void next() { self.begin_ = T(self.begin_ + 1); }

            ENABLE_FOREACH_ITERATOR(Iterator)
        };

        constexpr Iterator<const T> iterator() const { return {*this}; }

        constexpr Iterator<T> iterator() { return {*this}; }

        ENABLE_FOREACH_ITERABLE()
    };
}

// contiguous iterator
namespace tier0 {
    template<typename T, typename E>
    struct ContiguousIterator {
        ptr<const T> self_;
        Size index_;

        [[nodiscard]]
        constexpr Boolean hasNext() const { return index_ < self_->size(); }

        constexpr ref<E> get() const { return self_->data_[index_]; }

        constexpr void next() { index_ = index_ + 1; }

        ENABLE_FOREACH_ITERATOR(ContiguousIterator)
    };
}

// span
namespace tier0 {
    constexpr Size unbounded = Size(0xffffffff);

    template<typename T, Size N = unbounded>
    struct Span {
        using array_type = T[N];
        Native<ptr<T>> data_;
        Size size_;
    private:
        explicit Span(Native<ptr<T>> addr, Size size) : data_(addr), size_(size) {}

    public:
        static Span empty() { return Span(Native<ptr<T>>(), Size(0)); }

        explicit Span(mut_ref<array_type> array) : data_(array), size_(N) {}

        explicit Span(ptr<array_type> array) : data_(*array), size_(N) {}

        implicit Span(ref<Span> span) : data_(span.data_), size_(span.size_) {}

        template<Size M>
        requires (M >= N)
        implicit Span(ref<Span<T, M>> span) : data_(span.data_), size_(N) {}

        static Span unsafe(Native<ptr<T>> addr, Size size) { return Span(addr, size); }

        constexpr Boolean operator==(ref<Span> other) const { return compare(*this, other) == Order::Undefined; }

        static constexpr Order compare(ref<Span> a, ref<Span> b) {
            var i = Size(0);
            var nA = a.size();
            var nB = b.size();
            while (i < nA && i < nB) {
                let iA = a.get(Int(i));
                let iB = b.get(Int(i));
                if (iA < iB) {
                    return Order::Before;
                }
                if (iA > iB) {
                    return Order::After;
                }
                i = i + 1;
            }
            return nA.compareTo(nB);
        }

    public:

        [[nodiscard]]
        constexpr Size size() const { return size_; }

        constexpr ref<T> get(Int index) const {
            return data_[index];
        }

        constexpr mut_ref<T> get(Int index) {
            return data_[index];
        }

        constexpr void set(Int index, T value) {
            new(&data_[index]) T(move(value));
        }

        template<typename E>
        using Iterator = ContiguousIterator<Span, E>;

        constexpr Iterator<const T> iterator() const { return {this, Size(0)}; }

        constexpr Iterator<T> iterator() { return {this, Size(0)}; }

        ENABLE_FOREACH_ITERABLE()

        template<Size I>
        requires (I < N)
        [[nodiscard]]
        Span<T, N - I> offset() const { return Span<T, N - I>::unsafe(data_ + I, N - I); }

        [[nodiscard]] Span<T, unbounded> limit(Size limit) const {
            return Span<T, unbounded>::unsafe(data_, limit);
        }

        [[nodiscard]] Span<T, unbounded> offset(Size offset) const {
            return Span<T, unbounded>::unsafe(data_ + offset, size_ - offset);
        }
    };

    template<typename T>
    inline Order operator<=>(ref<Span<T>> a, ref<Span<T>> b) { return Span<T>::compare(a, b); }

    struct StringSpan {
        Span<const Byte> data_;

        explicit StringSpan(Span<const Byte> data) : data_(data) {}

        implicit StringSpan(cstring str)
                : data_(Span<const Byte>::unsafe(ptr<const Byte>::reinterpret(str), strlen(str))) {}

        constexpr Boolean operator==(ref<StringSpan> other) const { return (*this).data_ == other.data_; }

        Size size() const { return data_.size(); }

        static Size strlen(cstring str) {
            var ret = Size(0);
            while (str[ret] != 0) {
                ret = ret + 1;
            }
            return ret;
        }
    };

    inline Order operator<=>(ref<StringSpan> a, ref<StringSpan> b) { return a.data_ <=> b.data_; }
}

// array
namespace tier0 {
    template<typename T, Size N>
    struct Array {
        using array_type = T[N];
        array_type data_;

        [[nodiscard]]
        constexpr Size size() const { return N; }

        implicit constexpr Array() : data_() {}

        implicit constexpr Array(ref<array_type> data) {
            for (var i : Range<Size>::until(Size(0), N)) {
                data_[i] = data[i];
            }
        }

        Span<const T, N> asSpan() const { return Span<const T, N>(data_); }

        Span<T, N> asSpan() { return Span<T, N>(data_); }

        template<Size N2>
        constexpr Array<T, N + N2> concat(ref<Array<T, N2>> other) {
            let self = *this;
            var ret = Array<T, N + N2>();
            for (var i : Range<Size>::until(Size(0), N)) { ret.data_[i] = self.data_[i]; }
            for (var i : Range<Size>::until(Size(0), N2)) { ret.data_[N + i] = other.data_[i]; }
            return ret;
        }

        constexpr ref<T> get(Int index) const {
            return data_[index];
        }

        constexpr mut_ref<T> get(Int index) {
            return data_[index];
        }

        constexpr void set(Int index, T value) {
            new(&data_[index]) T(move(value));
        }

        template<typename E>
        using Iterator = ContiguousIterator<Array, E>;

        constexpr Iterator<const T> iterator() const { return {this, Size(0)}; }

        constexpr Iterator<T> iterator() { return {this, Size(0)}; }

        ENABLE_FOREACH_ITERABLE()
    };
}

// union
namespace tier0 {
    namespace detail {
        template<Size n>
        constexpr Size max(Array<Size, n> values) {
            var ret = Size(0);
            for (var it : values) {
                ret = it > ret ? it : ret;
            }
            return ret;
        }
    }

    template<Native<Size> size, Native<Size> align>
    struct AlignedStorage {
        alignas(align) Array<Byte, size> data_;
    };

    template<typename... Ts>
    struct AlignedUnionStorage : AlignedStorage<
            detail::max(Array<Size, Size(sizeof...(Ts))>({sizeof(Ts)...})),
            detail::max(Array<Size, Size(sizeof...(Ts))>({alignof(Ts)...}))
    > {
        // fixme: constexpr
        template<typename T>
        constexpr ref<T> get() const {
            Native<ptr<const T>> out;
            new(&out) Native<ptr<const void>>(&this->data_);
            return *out;
        }

        template<typename T>
        constexpr mut_ref<T> get() {
            Native<ptr<T>> out;
            new(&out) Native<ptr<void>>(&this->data_);
            return *out;
        }
    };

    template<typename... Ts>
    struct Union {
    private:
        AlignedUnionStorage<Ts...> data_;
        using types = TypeList<Ts...>;
    public:
        constexpr ~Union() {}

        template<Size i>
        constexpr void destroy() {
            using T = typename types::template get<i>;
            get<i>().~T();
        }

        explicit constexpr Union() {}

        implicit constexpr Union(movable<Union> other) : data_(move(other.data_)) {}

        Union copy() const {
            var ret = Union();
            ret.data_ = this->data_;
            return ret;
        }

        template<Native<Size> i>
        constexpr ref<typename types::template get<i>> get() const {
            using T = typename types::template get<i>;
            return data_.template get<T>();
        }

        template<Native<Size> i>
        constexpr mut_ref<typename types::template get<i>> get() {
            using T = typename types::template get<i>;
            return data_.template get<T>();
        }

        template<Native<Size> i>
        constexpr void set(movable<typename types::template get<i>> value) {
            using T = typename types::template get<i>;
            new(&get<i>()) T(move(value));
        }
    };
}

// unmanaged
namespace tier0 {
    template<typename T>
    struct Unmanaged {
    private:
        Union<T> data_;
    public:
        ~Unmanaged() {}

        void destroy() { get().~T(); }

        explicit Unmanaged() {}

        explicit Unmanaged(T value) {
            new(&get()) T(move(value));
        }

        implicit Unmanaged(movable<Unmanaged> other) {
            new(&get()) T(move(other.get()));
        }

        ref<T> get() const { return data_.template get<Size(0)>(); }

        mut_ref<T> get() { return data_.template get<Size(0)>(); }
    };
}

// optional
namespace tier0 {
    template<typename T>
    struct [[nodiscard]] ATTR_TYPESTATE_TYPE Optional {
    private:
        Union<T> data_;
        Boolean valueBit_;
    public:
        constexpr ~Optional() {
            if (valueBit_) {
                data_.template destroy<Size(0)>();
            }
        }

        ATTR_TYPESTATE_CTOR(unknown)

        implicit constexpr Optional() : valueBit_(false) {}

    public:
        implicit constexpr Optional(movable<Optional> other) : data_(move(other.data_)),
                                                               valueBit_(exchange(other.valueBit_, false)) {}

        explicit constexpr Optional(ref<Optional> other) : valueBit_(other.valueBit_) {
            if (other.hasValue()) {
                value() = other.value();
            }
        }

        // ATTR_TYPESTATE_CTOR(unconsumed)
        static constexpr Optional of(T value) {
            var ret = Optional();
            ret.data_.template set<Size(0)>(move(value));
            ret.valueBit_ = true;
            return ret;
        }

        ATTR_TYPESTATE_PRECONDITION(unknown)
        ATTR_TYPESTATE_ASSERTS(unconsumed)

        Native<Boolean> hasValue() const { return valueBit_ == Boolean(true); }

        ATTR_TYPESTATE_PRECONDITION(unconsumed)

        ref<T> value() const {
            assert(hasValue());
            return data_.template get<Size(0)>();
        }

        ATTR_TYPESTATE_PRECONDITION(unconsumed)

        mut_ref<T> value() {
            assert(hasValue());
            return data_.template get<Size(0)>();
        }

        // ATTR_TYPESTATE_CTOR(consumed)
        static Optional empty() {
            var ret = Optional();
            ret.valueBit_ = false;
            return ret;
        }
    };
}

// result
namespace tier0 {
    template<typename T, typename E>
    struct [[nodiscard]] ATTR_TYPESTATE_TYPE Result {
    private:
        Union<T, E> data_;
        Boolean errorBit_;
    public:
        ~Result() {
            if (!errorBit_) {
                data_.template destroy<Size(0)>();
            } else {
                data_.template destroy<Size(1)>();
            }
        }

    private:
        ATTR_TYPESTATE_CTOR(unknown)

        explicit Result() {}

    public:
        implicit Result(movable<Result> other) : data_(move(other.data_)), errorBit_(move(other.errorBit_)) {}

        // ATTR_TYPESTATE_CTOR(unconsumed)
        static Result value(T value) {
            var ret = Result();
            ret.data_.template set<Size(0)>(move(value));
            ret.errorBit_ = false;
            return ret;
        }

        ATTR_TYPESTATE_PRECONDITION(unknown)
        ATTR_TYPESTATE_ASSERTS(unconsumed)

        Native<Boolean> isValue() const { return errorBit_ == Boolean(false); }

        ATTR_TYPESTATE_PRECONDITION(unconsumed)

        ref<T> value() const { return data_.template get<Size(0)>(); }

        // ATTR_TYPESTATE_CTOR(consumed)
        static Result error(E error) {
            var ret = Result();
            ret.data_.template set<Size(1)>(move(error));
            ret.errorBit_ = true;
            return ret;
        }

        ATTR_TYPESTATE_PRECONDITION(unknown)
        ATTR_TYPESTATE_ASSERTS(consumed)

        Native<Boolean> isError() const { return errorBit_ == Boolean(true); }

        ATTR_TYPESTATE_PRECONDITION(consumed)

        ref<E> error() const { return data_.template get<Size(1)>(); }
    };
}

// variant
namespace tier0 {
    template<typename E, typename... Ts>
    struct Variant {
        static_assert(sizeof...(Ts) <= 255 - 1);
    private:
        Union<Ts...> data_;
        Byte active_;
        using types = TypeList<Ts...>;

        template<typename... Us>
        struct constructors;

        void construct(ref<Variant> other) {
            if (active_ == 0) {
                return;
            }
            apply<constructors, Enumerate<Ts...>>::invoke(*this, other);
        }

        template<typename... Us>
        struct destructors;

        void destroy() {
            if (active_ == 0) {
                return;
            }
            apply<destructors, Enumerate<Ts...>>::invoke(*this);
        }

    public:
        ~Variant() { destroy(); }

    private:
        explicit Variant() {}

    public:
        implicit Variant(movable<Variant> other) : data_(move(other.data_)), active_(other.active_) {}

        explicit Variant(ref<Variant> other) : active_(other.active_) {
            construct(other);
        }

        template<E i>
        static Variant of(typename types::template get<Native<Size>(i) - 1> value) {
            var ret = Variant();
            ret.template set<i>(move(value));
            return ret;
        }

        Variant copy() const {
            var ret = Variant();
            ret.active_ = this->active_;
            new(&ret.data_) Union<Ts...>(move(this->data_.copy()));
            return ret;
        }

        E index() const { return E(Native<Byte>(active_)); }

        template<E i>
        ref<typename types::template get<Native<Size>(i) - 1>> get() const {
            return data_.template get<Native<Size>(i) - 1>();
        }

        template<E i>
        mut_ref<typename types::template get<Native<Size>(i) - 1>> get() {
            return data_.template get<Native<Size>(i) - 1>();
        }

        template<E i>
        void set(movable<typename types::template get<Native<Size>(i) - 1>> value) {
            using T = typename types::template get<Native<Size>(i) - 1>;
            destroy();
            new(&get<i>()) T(move(value));
            active_ = Byte(Native<Size>(i));
        }

        template<E i>
        void set(ref<typename types::template get<Native<Size>(i) - 1>> value) {
            using T = typename types::template get<Native<Size>(i) - 1>;
            destroy();
            new(&get<i>()) T(value);
            active_ = Byte(Native<Size>(i));
        }

    private:
        template<typename... Us>
        struct constructors {
            using U = decltype(data_);

            static constexpr ptr<void(mut_ref<Variant> self, ref<Variant> other)> funcs[] = {
                    (+[](mut_ref<Variant> self, ref<Variant> other) {
                        constexpr var index = E(1 + Us::first::value);
                        self.set<index>(other.template get<index>());
                    })...
            };

            static constexpr void invoke(mut_ref<Variant> self, ref<Variant> other) {
                (*funcs[self.active_ - 1])(self, other);
            }
        };

        template<typename... Us>
        struct destructors {
            using U = decltype(data_);

            static constexpr mptr<U, void()> funcs[] = {&U::template destroy<Us::first::value>...};

            static constexpr void invoke(mut_ref<Variant> self) { funcs[self.active_ - 1].call(self.data_); }
        };
    };
}

// literal string
namespace tier0 {
    template<Size N>
    struct LiteralString;

    template<Native<Size> N>
    LiteralString(ref<Native<Char>[N]>) -> LiteralString<N>;

    template<Size N>
    struct LiteralString {
        using array_type = Native<Char>[N];
        array_type data_;

        explicit consteval LiteralString() : data_() {
            var n = size();
            for (var i : Range<Size>::until(Size(0), n)) {
                data_[i] = 0;
            }
            data_[n] = 0;
        }

        implicit consteval LiteralString(ref<Native<Char>[N]> str) : data_() {
            var n = size();
            for (var i : Range<Size>::until(Size(0), n)) {
                data_[i] = str[i];
            }
            data_[n] = 0;
        }

        [[nodiscard]] constexpr Size size() const { return N - 1; }

        template<Size begin, Size end = N>
        [[nodiscard]] consteval LiteralString<end - begin + 1> slice() const {
            const var n = end - begin;
            var ret = LiteralString<n + 1>();
            for (var i : Range<Size>::until(Size(0), n)) {
                ret.data_[i] = data_[begin + i];
            }
            ret.data_[n] = 0;
            return ret;
        }
    };

    template<LiteralString str>
    cstring global() {
        static constinit auto value = str;
        return value.data_;
    }
}

// source location
namespace tier0 {
    struct SourceLocation {
        const cstring file_;
        const cstring function_;
        const Int line_;

        static SourceLocation current(
                cstring file = __builtin_FILE(),
                cstring function = __builtin_FUNCTION(),
                Int line = Native<Int>(__builtin_LINE())
        ) {
            return {
                    file + sizeof(PROJECT_SOURCE_DIR) - 1,
                    function,
                    line,
            };
        }

    private:
        SourceLocation(cstring file, cstring function, Int line) : file_(file), function_(function), line_(line) {}
    };
}

// typename
namespace tier0 {
    namespace detail {
        template<typename T>
        consteval auto &RawTypeName() {
            return __PRETTY_FUNCTION__;
        }

        struct TypeNameFormat {
            Size leading_;
            Size trailing_;
        private:
            static consteval Boolean calculate(Native<ptr<TypeNameFormat>> ret) {
                using int_t = decltype(0);
                auto &needle = "int";
                auto &haystack = RawTypeName<int_t>();
                for (Size i = Size(0);; i = i + 1) {
                    if (haystack[i] == needle[0] and haystack[i + 1] == needle[1] and haystack[i + 2] == needle[2]) {
                        if (ret) {
                            ret->leading_ = i;
                            ret->trailing_ = (sizeof(haystack) - 1) - i - (sizeof(needle) - 1);
                        }
                        return true;
                    }
                }
            }

        public:
            static consteval TypeNameFormat get() {
                static_assert(TypeNameFormat::calculate(nullptr), "Unable to generate type name");
                auto ret = TypeNameFormat();
                TypeNameFormat::calculate(&ret);
                return ret;
            }
        };

        template<typename T>
        consteval auto TypeName() {
            constexpr auto format = TypeNameFormat::get();
            auto &raw = RawTypeName<T>();
            const auto n = (sizeof(raw) - 1) - (format.leading_ + format.trailing_);
            auto ret = Array<Native<Char>, n + 1>();
            auto o = 0;
            for (auto i : Range<remove_const < decltype(n)>>::until(0, n)) {
                auto c = raw[i + format.leading_];
                if (c == ' ' && raw[i + format.leading_ - 1] == '>') continue; // GCC avoids emitting `>>`
                ret.data_[o++] = c;
            }
            return ret;
        }
    }

    template<typename T>
    cstring TypeName() {
        static constinit auto name = detail::TypeName<T>();
        return name.data_;
    }
}

// constexpr strtok
namespace tier0::strtok {
    struct Range {
        Size begin_;
        Size end_;
    };

    template<LiteralString str>
    consteval Size count();

    namespace detail {
        template<LiteralString str>
        consteval Size countPlaceholders() {
            var ret = Size(0);
            for (var i = Size(0); i < str.size();) {
                if (str.data_[i] == '{' && str.data_[i + 1] == '}') {
                    ret = ret + 1;
                    i = i + 2;
                } else {
                    i = i + 1;
                }
            }
            return ret;
        }

        template<LiteralString str>
        consteval Size countCharactersUntilPlaceholder() {
            var i = Size(0);
            for (; i < str.size();) {
                if (str.data_[i] == '{' && str.data_[i + 1] == '}') {
                    break;
                } else {
                    i = i + 1;
                }
            }
            return i;
        }

        template<LiteralString str, typename T, Size n, typename F, Size offset>
        constexpr Array<T, n> forEachRange(F f) {
            constexpr var size = countCharactersUntilPlaceholder<str.template slice<offset>()>();
            var ret = Array<T, Size(1)>();
            ret.data_[0] = f.template operator()<Range{offset + 0, offset + size}>();
            if constexpr (n > 1) {
                var next = forEachRange<str, T, n - 1, F, offset + size + 2>(f);
                return ret.template concat(next);
            } else {
                return ret;
            }
        }

        template<LiteralString str, typename F>
        constexpr auto forEachRange(F f) {
            constexpr var n = count<str>();
            return forEachRange<str, decltype(f.template operator()<Range{}>()), n, F, Size(0)>(f);
        }
    }

    template<LiteralString str>
    consteval Size count() { return 1 + detail::countPlaceholders<str>(); }

    template<LiteralString str>
    constexpr Array<cstring, count<str>()> collect() {
        return detail::forEachRange<str>([]<Range r>() constexpr {
            return global<str.template slice<r.begin_, r.end_>()>();
        });
    }
}

// intrusive
namespace tier0 {
    template<typename T>
    struct IntrusiveLinks {
        Native<ptr<T>> prev_;
        Native<ptr<T>> next_;
    };

    template<typename T, mptr<T, IntrusiveLinks<T>> links>
    struct IntrusiveList {
    private:
        Native<ptr<T>> head_;
        Native<ptr<T>> tail_;
    public:
        void add(mut_ref<T> value) {
            let lValue = &links.get(value);
            let prev = lValue->prev_ = tail_;
            if (let lPrev = !prev ? nullptr : &links.get(*prev)) {
                lPrev->next_ = &value;
            } else {
                head_ = &value;
            }
            tail_ = &value;
        }

        void remove(mut_ref<T> value) {
            let lValue = &links.get(value);
            let prev = lValue->prev_;
            let next = lValue->next_;
            if (let lPrev = !prev ? nullptr : &links.get(*prev)) {
                lPrev->next_ = next;
            }
            if (let lNext = !next ? nullptr : &links.get(*next)) {
                lNext->prev_ = prev;
            }
            if (&value == head_) {
                head_ = next;
            }
            if (&value == tail_) {
                tail_ = prev;
            }
        }
    };
}
