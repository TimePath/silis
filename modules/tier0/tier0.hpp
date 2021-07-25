#pragma once

#define func auto
#define let const auto &
#define var auto

#define implicit

namespace tier0 {}
using namespace tier0;

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
    constexpr auto name = name##_s<MAP(METAFUNC_IMPL_DECLARE_FORWARD, METAFUNC_IMPL_DELIMITER_COMMA, args)>::value;
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

    namespace pack {
        // METAFUNC_TYPE(get, ((I, size_t), (types, typename...)))

        template<long unsigned int I, typename... types>
        struct get_s;

        template<long unsigned int I, typename... types>
        using get = typename get_s<I, types...>::type;

        METAFUNC_TYPE_IMPL(get, (0, T, types...), ((T, typename),(types, typename...)), T)
        METAFUNC_TYPE_IMPL(get, (I, T, types...), ((I, long unsigned int), (T, typename),(types, typename...)),
                           get<I - 1 METAFUNC_IMPL_DELIMITER_COMMA() types...>)
    }

    template<typename T, long unsigned int N>
    struct SizedArray {
        T data[N];

        SizedArray() : data() {}

        implicit SizedArray(T (&data)[N]) : data(data) {}
    };
}

// memory model
namespace tier0 {
    using cstring = const char *;

    template<typename T>
    using ptr = T *;

    template<typename T>
    using mut_ref = T &;

    template<typename T>
    using ref = const T &;

    template<typename T>
    using movable = T &&;

    template<typename T>
    constexpr movable<typename remove_reference_s<T>::type> move(movable<T> value) noexcept {
        return static_cast<movable<typename remove_reference_s<T>::type>>(value);
    }
}

// primitives
namespace tier0 {
    namespace detail {
        METAFUNC_VALUE(is_arithmetic, ((T, typename)))

        template<typename T>
        struct is_arithmetic_s {
            static constexpr auto value = is_same < bool, T> || is_same<char, T>
            || is_same<short, T> || is_same<int, T> || is_same<long, T>
            || is_same<float, T> || is_same<double, T>;
        };

        template<typename T>
        struct WordTraits {
        };

        template<typename T>
        concept is_word = WordTraits<T>::isWord;

        template<typename T>
        class Word {
        public:
            T value;

            constexpr ~Word() noexcept {}

            constexpr Word() noexcept: Word(0) {}

            /** Exact primitive match */
            template<typename T2>
            requires is_same<T, T2>
            implicit constexpr Word(T2 value) noexcept : value(value) {}

            /** Inexact primitive match */
            template<typename U>
            requires (!is_same < T, U > && is_arithmetic < U >)
            explicit constexpr Word(U value) noexcept : value((T) value) {}

            /** Copy */
            implicit constexpr Word(ref<Word> other) noexcept {
                *this = other;
            }

            constexpr mut_ref<Word> operator=(ref<Word> other) noexcept {
                if (&other == this) return *this;
                value = other.value;
                return *this;
            }

            /** Cast from */
            template<typename Other>
            requires is_word<Other>
            explicit Word(Other word) noexcept : Word((typename WordTraits<Other>::native) word) {}

            /** Cast to */
            implicit constexpr operator T() const noexcept { return value; }
        };

        template<typename T>
        struct WordTraits<Word<T>> {
            static const bool isWord = true;
            using native = T;
        };
    }

    template<typename T> requires detail::is_word<T>
    using Native = typename detail::WordTraits<T>::native;

    struct Unit {
        explicit Unit() = default;
    };

#define WORD(native) detail::Word<native>
    using Boolean = WORD(bool);
    using Char = WORD(char);
    using Byte = WORD(signed char);
    using UByte = WORD(unsigned char);
    using Short = WORD(short signed int);
    using UShort = WORD(short unsigned int);
    using Int = WORD(signed int);
    using UInt = WORD(unsigned int);
    using Long = WORD(long signed int);
    using ULong = WORD(long unsigned int);
    using Float = WORD(float);
    using Double = WORD(double);
#undef WORD

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

// typename
namespace tier0 {
    namespace detail {
        template<typename T>
        constexpr auto &TypeNameRaw() {
            return __PRETTY_FUNCTION__;
        }

        struct TypeNameFormat {
            using size_t = decltype(sizeof 0);
            size_t leading;
            size_t trailing;

            static constexpr Native<Boolean> calculate(ptr<TypeNameFormat> ret) {
                using int_t = decltype(0);
                auto &needle = "int";
                auto &haystack = TypeNameRaw<int_t>();
                for (auto i = 0;; ++i) {
                    if (haystack[i] == needle[0] && haystack[i + 1] == needle[1] && haystack[i + 2] == needle[2]) {
                        if (ret) {
                            ret->leading = i;
                            ret->trailing = (sizeof(haystack) - 1) - i - (sizeof(needle) - 1);
                        }
                        return true;
                    }
                }
                return false;
            }

            static constexpr TypeNameFormat calculate() {
                auto ret = TypeNameFormat();
                calculate(&ret);
                return ret;
            }
        };

        constexpr auto typeNameFormat = [] {
            static_assert(TypeNameFormat::calculate(nullptr), "Unable to generate type name");
            return TypeNameFormat::calculate();
        }();

        template<typename T>
        constexpr auto TypeName() {
            auto &raw = TypeNameRaw<T>();
            const auto n = (sizeof(raw) - 1) - (typeNameFormat.leading + typeNameFormat.trailing);
            auto ret = SizedArray<Native<Char>, n + 1>{};
            for (auto i = (decltype(n)) 0; i < n; ++i) {
                ret.data[i] = raw[i + typeNameFormat.leading];
            }
            return ret;
        }
    }

    template<typename T>
    cstring TypeName() {
        static auto name = detail::TypeName<T>();
        return name.data;
    }
}

// https://en.cppreference.com/w/cpp/named_req
namespace tier0 {
    class DisableDefaultConstructible {
        DisableDefaultConstructible() = delete;

    protected:
        DisableDefaultConstructible(Unit) {}
    };

    class DisableDestructible {
    protected:
        ~DisableDestructible() = delete;
    };

    class DisableCopyConstructible {
        DisableCopyConstructible(const DisableCopyConstructible &) = delete;

    protected:
        DisableCopyConstructible(Unit) {}
    };

    class DisableCopyAssignable {
        DisableCopyAssignable &operator=(const DisableCopyAssignable &) = delete;
    };

    class DisableMoveConstructible {
        DisableMoveConstructible(DisableMoveConstructible &&) = delete;

    protected:
        DisableMoveConstructible(Unit) {}
    };

    class DisableMoveAssignable {
        DisableMoveAssignable &operator=(DisableMoveAssignable &&) = delete;
    };
}

// structured binding
namespace tier0 {
    template<typename ... elements>
    struct tuple_elements {
        static constexpr Native<ULong> size = sizeof...(elements);

        template<Native<ULong> I>
        using type = typename pack::get<I, elements...>::type;

        template<Native<ULong> I, typename T>
        static type<I> &value(T &self) { return pack::get<I, elements...>::value(self); }

        template<Native<ULong> I, typename T>
        static type<I> const &value(T const &self) { return pack::get<I, elements...>::value(self); }
    };

    namespace {
        template<typename T, T v>
        struct mptr_info;

        template<typename T, typename R, R T::*v>
        struct mptr_info<R T::*, v> {
            using type = R;

            static type &value(T &self) { return self.*v; }

            static type const &value(T const &self) { return self.*v; }
        };
    }

    template<typename... types>
    struct tuple_elements_builder {
        template<auto mptr>
        using add = tuple_elements_builder<types..., mptr_info<decltype(mptr), mptr>>;

        using build = tuple_elements<types...>;
    };

    template<typename T>
    struct tuple_traits_s;
    template<typename T>
    using tuple_traits = tuple_traits_s<remove_reference<T>>;

    template<typename T>
    struct tuple_traits_s {
        using delegate = typename T::elements;

        static constexpr Native<ULong> size = delegate::size;

        template<Native<ULong> I>
        using type = typename delegate::template type<I>;

        template<Native<ULong> I>
        static type<I> &value(T &self) { return delegate::template value<I, T>(self); }

        template<Native<ULong> I>
        static type<I> const &value(T const &self) { return delegate::template value<I, T>(self); }
    };

    template<typename T>
    struct tuple_traits_s<const T> {
        using delegate = tuple_traits<remove_const < T>>;

        static constexpr Native<ULong> size = delegate::size;

        template<Native<ULong> I>
        using type = add_const<typename delegate::template type<I>>;

        template<Native<ULong> I>
        static type<I> &value(T const &self) { return delegate::template value<I>(self); }
    };
}

namespace std {
    template<typename T>
    struct tuple_size;

    template<Native<ULong> I, typename T>
    struct tuple_element;
}

template<typename T>
struct std::tuple_size {
    static constexpr Native<ULong> value = tuple_traits<T>::size;
};

template<Native<ULong> I, typename T>
struct std::tuple_element {
    using type = typename tuple_traits<T>::template type<I>;
};

template<Native<ULong> I, typename T>
typename std::tuple_element<I, T>::type &get(T &self) {
    return tuple_traits<T>::template value<I>(self);
}
