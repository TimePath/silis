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

namespace tier0 {
    [[noreturn]]
    inline void die() {
        (void) *static_cast<char *>(nullptr);
        __builtin_unreachable();
    }
}

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
    using cstring = const char *;

    template<typename T>
    using ptr = const T *;

    template<typename T>
    using mut_ptr = T *;

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
            T wordValue;

            constexpr ~Word() {}

            explicit constexpr Word() : Word(0) {}

            /** Exact primitive match */
            template<typename T2>
            requires is_same<T, T2>
            implicit constexpr Word(T2 value) : wordValue(value) {}

            /** Inexact primitive match */
            template<typename U>
            requires (!is_same < T, U > and is_arithmetic < U >)
            explicit constexpr Word(U value) : wordValue(T(value)) {}

            /** Copy */
            implicit constexpr Word(ref<Word> other) {
                *this = other;
            }

            constexpr mut_ref<Word> operator=(ref<Word> other) {
                wordValue = other.wordValue;
                return *this;
            }

            /** Cast from */
            template<typename Other>
            requires WordTraits<Other>::isWord
            explicit Word(Other word) : Word(typename WordTraits<Other>::native(word)) {}

            /** Cast to */
            implicit constexpr operator T() const { return wordValue; }

            // operators

            constexpr bool operator==(ref<Word> other) const { return (*this).wordValue == other.wordValue; }
        };

        template<typename T> requires is_base_of<WordTag, T>
        struct WordTraits<T> {
            static const bool isWord = true;
            using native = decltype(ptr<T>(nullptr)->wordValue);
        };
    }

    template<typename T> requires detail::WordTraits<T>::isWord
    using Native = typename detail::WordTraits<T>::native;

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

// placement new

tier0::mut_ptr<void> operator new(tier0::Native<tier0::Size> count, tier0::mut_ptr<void> place) noexcept;

tier0::mut_ptr<void> operator new[](tier0::Native<tier0::Size> count, tier0::mut_ptr<void> place) noexcept;

void operator delete(tier0::mut_ptr<void> ptr, tier0::mut_ptr<void> place) noexcept;

void operator delete[](tier0::mut_ptr<void> ptr, tier0::mut_ptr<void> place) noexcept;

#define IMPLEMENTS_PLACEMENT 1

#if IMPLEMENTS_PLACEMENT

inline tier0::mut_ptr<void> operator new(tier0::Native<tier0::Size> count, tier0::mut_ptr<void> place) noexcept {
    (void) count;
    return place;
}

inline tier0::mut_ptr<void> operator new[](tier0::Native<tier0::Size> count, tier0::mut_ptr<void> place) noexcept {
    (void) count;
    return place;
}

inline void operator delete(tier0::mut_ptr<void> ptr, tier0::mut_ptr<void> place) noexcept {
    (void) ptr;
    (void) place;
}

inline void operator delete[](tier0::mut_ptr<void> ptr, tier0::mut_ptr<void> place) noexcept {
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
    namespace detail {
        // METAFUNC_TYPE(get, ((I, Native<Size>), (Ts, typename...)))

        template<Native<Size> I, typename... Ts>
        struct get_s;

        template<Native<Size> I, typename... Ts>
        using get = typename get_s<I, Ts...>::type;

        METAFUNC_TYPE_IMPL(get, (0, T, Ts...), ((T, typename),(Ts, typename...)), T)
        METAFUNC_TYPE_IMPL(get, (I, T, Ts...), ((I, Native<Size>), (T, typename),(Ts, typename...)),
                           get<I - 1 METAFUNC_IMPL_DELIMITER_COMMA() Ts...>)
    }

    template<typename... Ts>
    struct TypeList {
        template<Size I>
        using get = detail::get<I, Ts...>;

        METAFUNC_TYPE(concat, ((U, typename)))
        METAFUNC_TYPE_IMPL(concat, (TypeList<Us...>), ((Us, typename...)),
                           TypeList<Ts... METAFUNC_IMPL_DELIMITER_COMMA() Us...>)

        template<template<typename> typename F>
        using map = TypeList<F<Ts>...>;
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

    template<Size i = Size(0)>
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
}

// structured binding
namespace tier0 {
    namespace detail {
        template<typename T, T v>
        struct property;

        template<typename T, typename R, R T::*v>
        struct property<R T::*, v> {
            using type = R;

            static ref<type> get(ref<T> self) { return self.*v; }

            static mut_ref<type> get(mut_ref<T> self) { return self.*v; }
        };
    }

    template<typename... elements>
    struct tuple_elements {
        static constexpr Size size = sizeof...(elements);

        using types = TypeList<elements...>;

        template<Native<Size> I>
        using type = typename types::template get<I>::type;

        template<Native<Size> I, typename T>
        static ref<type<I>> get(ref<T> self) { return types::template get<I>::get(self); }

        template<Native<Size> I, typename T>
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

        template<Native<Size> I>
        using type = typename delegate::template type<I>;

        template<Native<Size> I>
        static ref<type<I>> get(ref<T> self) { return delegate::template get<I, T>(self); }

        template<Native<Size> I>
        static mut_ref<type<I>> get(mut_ref<T> self) { return delegate::template get<I, T>(self); }
    };

    template<typename T>
    struct tuple_traits_s<const T> {
        using delegate = tuple_traits<remove_const < T>>;

        static constexpr Size size = delegate::size;

        template<Native<Size> I>
        using type = add_const<typename delegate::template type<I>>;

        template<Native<Size> I>
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

    template<tier0::Native<tier0::Size> i, typename T>
    struct tuple_element;

    template<typename T>
    struct tuple_size {
        static constexpr tier0::Native<tier0::Size> value = tier0::tuple_traits<T>::size;
    };

    template<tier0::Native<tier0::Size> i, typename T>
    struct tuple_element {
        using type = typename tier0::tuple_traits<T>::template type<i>;
    };
}

// tuple
namespace tier0 {
    template<Size i, typename T>
    struct TupleLeaf {
        T value;
    };

    template<typename... Ts>
    struct TupleStorage : Ts ... {
    };

    template<typename T>
    using ToTupleLeaf = TupleLeaf<T::first::value, typename T::second::type>;

    template<typename... Ts>
    struct Tuple {
    private:
        apply<TupleStorage, typename collect<ZipIterator<CounterIterator<>, PackIterator<Ts...>>>::template map<ToTupleLeaf>> data;
    public:
        implicit Tuple(Ts... args) : data{{args}...} {
        }

        struct [[maybe_unused]] elements {
            static constexpr Size size = sizeof...(Ts);

            template<Size I>
            using type = typename TypeList<Ts...>::template get<I>;

            template<Size I, typename T>
            static ref<type<I>> get(ref<T> self) { return self.data.TupleLeaf<I, type<I>>::value; }

            template<Size I, typename T>
            static mut_ref<type<I>> get(mut_ref<T> self) { return self.data.TupleLeaf<I, type<I>>::value; }
        };

        ENABLE_STRUCTURED_BINDING(Tuple)
    };

    template<typename... Ts>
    Tuple(Ts... args) -> Tuple<Ts...>;
}

// iterable

#define ENABLE_FOREACH_ITERABLE() \
    constexpr mut_ptr<void> end() const { return nullptr; } \
    constexpr auto begin() const { return iterator(); } \
    constexpr auto begin() { return iterator(); } \
    /**/

#define ENABLE_FOREACH_ITERATOR(T) \
    constexpr mut_ptr<void> end() const { return nullptr; } \
    constexpr ref<T> begin() const { return *this; } \
    constexpr mut_ref<T> begin() { return *this; } \
    /**/

// iterator

template<typename Iterator>
inline constexpr tier0::Boolean operator!=(tier0::ref<Iterator> self, tier0::mut_ptr<void>) { return self.hasNext(); }

template<typename Iterator>
inline constexpr auto &operator*(tier0::ref<Iterator> self) { return self.get(); }

template<typename Iterator>
inline constexpr void operator++(tier0::mut_ref<Iterator> self) { self.next(); }

// range
namespace tier0 {
    template<typename T>
    struct Range {
    private:
        T _begin;
        T _end;

        constexpr Range(T begin, T end) : _begin(begin), _end(end) {}

    public:
        static constexpr Range until(T begin, T end) { return {begin, end}; }

        constexpr Boolean contains(T i) { return (i >= _begin and i < _end); }

        template<typename E>
        struct Iterator {
            Range self;

            constexpr Boolean hasNext() const { return self._begin < self._end; }

            constexpr ref<E> get() const { return self._begin; }

            constexpr void next() { self._begin = self._begin + 1; }

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
        ptr<T> self;
        Size idx;

        [[nodiscard]]
        constexpr Boolean hasNext() const { return idx < self->size(); }

        constexpr ref<E> get() const { return self->_data[idx]; }

        constexpr void next() { idx = idx + 1; }

        ENABLE_FOREACH_ITERATOR(ContiguousIterator)
    };
}

// span
namespace tier0 {
    namespace detail {
        template<typename T, Native<Size> I>
        using Arr = T[I];

        template<typename T, Native<Size> N, Native<Size> M>
        static ref<Arr<T, N>> array_cast(ref<Arr<T, M>> arr) {
            return *ptr<Arr<T, N>>(ptr<void>(&arr));
        }

        template<typename T, Native<Size> N, Native<Size> M>
        static mut_ref<Arr<T, N>> array_cast(mut_ref<Arr<T, M>> arr) {
            return *mut_ptr<Arr<T, N>>(mut_ptr<void>(&arr));
        }
    }

    template<typename T, Native<Size> N>
    struct Span {
        using array_type = T[N];
        mut_ref<array_type> _data;

        [[nodiscard]]
        constexpr Size size() const { return N; }

        explicit Span(mut_ref<array_type> array) : _data(array) {}

        explicit Span(mut_ptr<array_type> array) : _data(*array) {}

        implicit Span(ref<Span<T, N>> span) : _data(span._data) {}

        template<Native<Size> M>
        requires (M >= N)
        implicit Span(ref<Span<T, M>> span) : Span(detail::array_cast<T, N, M>(span._data)) {}

        template<typename E>
        using Iterator = ContiguousIterator<Span, E>;

        constexpr Iterator<const T> iterator() const { return {this, Size(0)}; }

        constexpr Iterator<T> iterator() { return {this, Size(0)}; }

        ENABLE_FOREACH_ITERABLE()

        template<Native<Size> I>
        requires (I < N)
        [[nodiscard]]
        Span<T, N - I> offset() const { return Span{mut_ptr<array_type>(_data + I)}; }
    };
}

// array
namespace tier0 {
    template<typename T, Native<Size> N>
    struct Array {
        using array_type = T[N];
        array_type _data;

        [[nodiscard]]
        constexpr Size size() const { return N; }

        implicit constexpr Array() : _data() {}

        implicit constexpr Array(ref<array_type> data) {
            for (var i : Range<Size>::until(Size(0), N)) {
                this->_data[i] = data[i];
            }
        }

        Span<const T, N> asSpan() const { return Span(this->_data); }

        Span<T, N> asSpan() { return Span(this->_data); }

        template<Native<Size> N2>
        constexpr Array<T, N + N2> concat(ref<Array<T, N2>> other) {
            let self = *this;
            var ret = Array<T, N + N2>();
            for (var i : Range<Size>::until(Size(0), N)) { ret._data[i] = self._data[i]; }
            for (var i : Range<Size>::until(Size(0), N2)) { ret._data[N + i] = other._data[i]; }
            return ret;
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
        template<Native<Size> n>
        constexpr Native<Size> max(Array<Native<Size>, n> values) {
            var ret = Native<Size>(0);
            for (var it : values) {
                ret = it > ret ? it : ret;
            }
            return ret;
        }
    }

    template<Size size, Native<Size> align>
    struct AlignedStorage {
        alignas(align) Array<Byte, size> bytes;
    };

    template<typename... Ts>
    struct AlignedUnionStorage : AlignedStorage<
            detail::max(Array<Native<Size>, sizeof...(Ts)>({sizeof(Ts)...})),
            detail::max(Array<Native<Size>, sizeof...(Ts)>({alignof(Ts)...}))
    > {
    };

    template<typename... Ts>
    struct Union {
    private:
        AlignedUnionStorage<Ts...> u;
        using types = TypeList<Ts...>;
    public:
        ~Union() {}

        template<Native<Size> i>
        void destroy() {
            using T = typename types::template get<i>;
            get<i>().~T();
        }

        explicit Union() {}

        implicit Union(movable<Union> other) : u(move(other.u)) {}

        template<Native<Size> i>
        ref<typename types::template get<i>> get() const {
            using T = typename types::template get<i>;
            union {
                ptr<void> in;
                ptr<T> out;
            } pun;
            pun.in = &u.bytes;
            return *pun.out;
        }

        template<Native<Size> i>
        mut_ref<typename types::template get<i>> get() {
            using T = typename types::template get<i>;
            union {
                mut_ptr<void> in;
                mut_ptr<T> out;
            } pun;
            pun.in = &u.bytes;
            return *pun.out;
        }

        template<Native<Size> i>
        void set(movable<typename types::template get<i>> value) {
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
        Union<T> u;
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

        ref<T> get() const { return u.template get<0>(); }

        mut_ref<T> get() { return u.template get<0>(); }
    };
}

// optional
namespace tier0 {
    template<typename T>
    struct [[nodiscard]] ATTR_TYPESTATE_TYPE Optional {
    private:
        Union<T> u;
        Boolean valueBit;
    public:
        ~Optional() {
            if (valueBit) {
                u.template destroy<0>();
            }
        }

    private:
        ATTR_TYPESTATE_CTOR(unknown)

        explicit Optional() {}

    public:
        implicit Optional(movable<Optional> other) : u(move(other.u)), valueBit(move(other.valueBit)) {}

        // ATTR_TYPESTATE_CTOR(unconsumed)
        static Optional of(T value) {
            var ret = Optional();
            ret.u.template set<0>(move(value));
            ret.valueBit = true;
            return ret;
        }

        ATTR_TYPESTATE_PRECONDITION(unknown)
        ATTR_TYPESTATE_ASSERTS(unconsumed)

        Native<Boolean> hasValue() const { return valueBit == Boolean(true); }

        ATTR_TYPESTATE_PRECONDITION(unconsumed)

        ref<T> value() const { return u.template get<0>(); }

        // ATTR_TYPESTATE_CTOR(consumed)
        static Optional empty() {
            var ret = Optional();
            ret.valueBit = false;
            return ret;
        }
    };
}

// result
namespace tier0 {
    template<typename T, typename E>
    struct [[nodiscard]] ATTR_TYPESTATE_TYPE Result {
    private:
        Union<T, E> u;
        Boolean errorBit;
    public:
        ~Result() {
            if (!errorBit) {
                u.template destroy<0>();
            } else {
                u.template destroy<1>();
            }
        }

    private:
        ATTR_TYPESTATE_CTOR(unknown)

        explicit Result() {}

    public:
        implicit Result(movable<Result> other) : u(move(other.u)), errorBit(move(other.errorBit)) {}

        // ATTR_TYPESTATE_CTOR(unconsumed)
        static Result value(T value) {
            var ret = Result();
            ret.u.template set<0>(move(value));
            ret.errorBit = false;
            return ret;
        }

        ATTR_TYPESTATE_PRECONDITION(unknown)
        ATTR_TYPESTATE_ASSERTS(unconsumed)

        Native<Boolean> isValue() const { return errorBit == Boolean(false); }

        ATTR_TYPESTATE_PRECONDITION(unconsumed)

        ref<T> value() const { return u.template get<0>(); }

        // ATTR_TYPESTATE_CTOR(consumed)
        static Result error(E error) {
            var ret = Result();
            ret.u.template set<1>(move(error));
            ret.errorBit = true;
            return ret;
        }

        ATTR_TYPESTATE_PRECONDITION(unknown)
        ATTR_TYPESTATE_ASSERTS(consumed)

        Native<Boolean> isError() const { return errorBit == Boolean(true); }

        ATTR_TYPESTATE_PRECONDITION(consumed)

        ref<E> error() const { return u.template get<1>(); }
    };
}

// variant
namespace tier0 {
    template<typename... Ts>
    struct Variant {
        static_assert(sizeof...(Ts) <= 255 - 1);
    private:
        Union<Ts...> u;
        Byte active;
        using types = TypeList<Ts...>;

        template<typename... Us>
        struct destructors {
            using U = decltype(u);

            template<typename T, typename R>
            using mptr = R T::*;

            static constexpr mptr<U, void()> funcs[] = {&U::template destroy<Us::first::value>...};

            static constexpr void invoke(mut_ref<Variant> self) { (self.u.*funcs[self.active - 1])(); }
        };

        void destroy() {
            if (active == 0) {
                return;
            }
            apply<destructors, collect<ZipIterator<CounterIterator<>, PackIterator<Ts...>>>>::invoke(*this);
        }

    public:
        ~Variant() { destroy(); }

    private:
        explicit Variant() {}

    public:
        implicit Variant(movable<Variant> other) : u(move(other.u)), active(move(other.active)) {}

        template<Native<Size> i>
        static Variant of(typename types::template get<i> value) {
            var ret = Variant();
            ret.template set<i>(move(value));
            return ret;
        }

        template<Native<Size> i>
        ref<typename types::template get<i>> get() const {
            return u.template get<i>();
        }

        template<Native<Size> i>
        mut_ref<typename types::template get<i>> get() {
            return u.template get<i>();
        }

        template<Native<Size> i>
        void set(movable<typename types::template get<i>> value) {
            using T = typename types::template get<i>;
            destroy();
            new(&get<i>()) T(move(value));
            active = Byte(1 + i);
        }
    };
}

// literal string
namespace tier0 {
    template<Native<Size> N>
    struct LiteralString {
        using array_type = Native<Char>[N];
        array_type data;

        [[nodiscard]]
        constexpr Size size() const { return N - 1; }

        explicit consteval LiteralString() : data() {
            var n = size();
            for (var i : Range<Size>::until(Size(0), n)) {
                data[i] = 0;
            }
            data[n] = 0;
        }

        implicit consteval LiteralString(ref<Native<Char>[N]> str) : data() {
            var n = size();
            for (var i : Range<Size>::until(Size(0), n)) {
                data[i] = str[i];
            }
            data[n] = 0;
        }

        template<Size begin = Size(0), Size end = N>
        [[nodiscard]]
        consteval LiteralString<end - begin + 1> slice() const {
            const var n = end - begin;
            var ret = LiteralString<n + 1>();
            for (var i : Range<Size>::until(Size(0), n)) {
                ret.data[i] = data[begin + i];
            }
            ret.data[n] = 0;
            return ret;
        }
    };

    template<Native<Size> N>
    LiteralString(ref<Native<Char>[N]> str) -> LiteralString<N>;

    template<LiteralString str>
    cstring global() {
        static constinit auto value = str;
        return value.data;
    }
}

// source location
namespace tier0 {
    struct SourceLocation {
        const cstring _file;
        const cstring _function;
        const Int _line;

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
        SourceLocation(cstring file, cstring function, Int line) : _file(file), _function(function), _line(line) {}
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
            Size leading;
            Size trailing;
        private:
            static consteval Boolean calculate(mut_ptr<TypeNameFormat> ret) {
                using int_t = decltype(0);
                auto &needle = "int";
                auto &haystack = RawTypeName<int_t>();
                for (Size i = Size(0);; i = i + 1) {
                    if (haystack[i] == needle[0] and haystack[i + 1] == needle[1] and haystack[i + 2] == needle[2]) {
                        if (ret) {
                            ret->leading = i;
                            ret->trailing = (sizeof(haystack) - 1) - i - (sizeof(needle) - 1);
                        }
                        return true;
                    }
                }
                return false;
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
            const auto n = (sizeof(raw) - 1) - (format.leading + format.trailing);
            auto ret = Array<Native<Char>, n + 1>();
            for (auto i : Range<remove_const < decltype(n)>>::until(0, n)) {
                ret._data[i] = raw[i + format.leading];
            }
            return ret;
        }
    }

    template<typename T>
    cstring TypeName() {
        static constinit auto name = detail::TypeName<T>();
        return name._data;
    }
}

// constexpr strtok
namespace tier0::strtok {
    struct Range {
        Size begin;
        Size end;
    };

    template<LiteralString str>
    consteval Size count();

    namespace detail {
        template<LiteralString str>
        consteval Native<Size> countPlaceholders() {
            var ret = Native<Size>(0);
            for (var i = Size(0); i < str.size();) {
                if (str.data[i] == '{' && str.data[i + 1] == '}') {
                    ret = ret + 1;
                    i = i + 2;
                } else {
                    i = i + 1;
                }
            }
            return ret;
        }

        template<LiteralString str>
        consteval Native<Size> countCharactersUntilPlaceholder() {
            var i = Size(0);
            for (; i < str.size();) {
                if (str.data[i] == '{' && str.data[i + 1] == '}') {
                    break;
                } else {
                    i = i + 1;
                }
            }
            return i;
        }

        template<LiteralString str, typename T, Size n, typename F, Size offset>
        constexpr Array<T, n> forEachRange(F f) {
            const var size = countCharactersUntilPlaceholder<str.template slice<offset>()>();
            var ret = Array<T, 1>();
            ret._data[0] = f.template operator()<Range{offset + 0, offset + size}>();
            if constexpr (n > 1) {
                var next = forEachRange<str, T, n - 1, F, offset + size + 2>(f);
                return ret.template concat(next);
            } else {
                return ret;
            }
        }

        template<LiteralString str, typename F>
        constexpr auto forEachRange(F f) {
            const var n = Native<Size>(count<str>());
            return forEachRange<str, decltype(f.template operator()<Range{}>()), n, F, Size(0)>(f);
        }
    }

    template<LiteralString str>
    consteval Size count() { return 1 + detail::countPlaceholders<str>(); }

    template<LiteralString str>
    constexpr Array<cstring, count<str>()> collect() {
        return detail::forEachRange<str>([]<Range r>() constexpr {
            return global<str.template slice<r.begin, r.end>()>();
        });
    }
}

namespace tier0 {
    template<Size i = Size(0), typename T, Size n = tuple_traits<T>::size, typename F, typename A>
    constexpr void forEach(ref<T> tuple, F f, A acc) {
        if constexpr (i < n) {
            acc = f(acc, tuple.template get<i>());
            forEach<i + 1, T, n, F>(tuple, f, acc);
        }
    }
}
