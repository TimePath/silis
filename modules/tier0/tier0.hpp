#pragma once

#define func auto
#define let const auto &
#define var auto

#define implicit

namespace tier0 {}
using namespace tier0;

namespace tier0 {
    [[noreturn]]
    inline void die() {
        (void) *(char *) nullptr;
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
        // METAFUNC_TYPE(get, ((I, Native<Size>), (types, typename...)))

        template<long unsigned int I, typename... types>
        struct get_s;

        template<long unsigned int I, typename... types>
        using get = typename get_s<I, types...>::type;

        METAFUNC_TYPE_IMPL(get, (0, T, types...), ((T, typename),(types, typename...)), T)
        METAFUNC_TYPE_IMPL(get, (I, T, types...), ((I, long unsigned int), (T, typename),(types, typename...)),
                           get<I - 1 METAFUNC_IMPL_DELIMITER_COMMA() types...>)
    }
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
        return static_cast<movable<remove_reference<T>>>(value);
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
        struct WordTraits {
        };

        template<typename T>
        concept is_word = WordTraits<T>::isWord;

        template<typename T> requires is_arithmetic<T>
        struct Word {
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
            explicit constexpr Word(U value) : wordValue((T) value) {}

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
            requires is_word<Other>
            explicit Word(Other word) : Word((typename WordTraits<Other>::native) word) {}

            /** Cast to */
            implicit constexpr operator T() const { return wordValue; }

            // operators

            constexpr bool operator==(ref<Word> other) { return (*this).wordValue == other.wordValue; }
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

    using Boolean = detail::Word<bool>;
    using Char = detail::Word<char>;
    using Byte = detail::Word<signed char>;
    using UByte = detail::Word<unsigned char>;
    using Short = detail::Word<short signed int>;
    using UShort = detail::Word<short unsigned int>;
    using Int = detail::Word<signed int>;
    using UInt = detail::Word<unsigned int>;
    using Long = detail::Word<long signed int>;
    using ULong = detail::Word<long unsigned int>;
    using Float = detail::Word<float>;
    using Double = detail::Word<double>;
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

// array
namespace tier0 {
    template<typename T, Native<Size> N>
    struct SizedArray {
        using array_type = T[N];
        array_type data;

        implicit constexpr SizedArray() : data() {}

        implicit constexpr SizedArray(ref<array_type> data) : data(data) {}
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
            };
        };

        template<typename T>
        consteval auto TypeName() {
            constexpr auto format = TypeNameFormat::get();
            auto &raw = RawTypeName<T>();
            const auto n = (sizeof(raw) - 1) - (format.leading + format.trailing);
            auto ret = SizedArray<Native<Char>, n + 1>{};
            for (auto i = (decltype(n)) 0; i < n; ++i) {
                ret.data[i] = raw[i + format.leading];
            }
            return ret;
        }
    }

    template<typename T>
    cstring TypeName() {
        static constinit auto name = detail::TypeName<T>();
        return name.data;
    }
}

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

    template<typename ... elements>
    struct tuple_elements {
        static constexpr Size size = sizeof...(elements);

        template<Native<Size> I>
        using type = typename pack::get<I, elements...>::type;

        template<Native<Size> I, typename T>
        static ref<type<I>> get(ref<T> self) { return pack::get<I, elements...>::get(self); }

        template<Native<Size> I, typename T>
        static mut_ref<type<I>> get(mut_ref<T> self) { return pack::get<I, elements...>::get(self); }
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
}

namespace std {
    template<typename T>
    struct tuple_size;

    template<Native<Size> I, typename T>
    struct tuple_element;

    template<typename T>
    struct tuple_size {
        static constexpr Native<Size> value = tuple_traits<T>::size;
    };

    template<Native<Size> I, typename T>
    struct tuple_element {
        using type = typename tuple_traits<T>::template type<I>;
    };
}

template<Native<Size> I, typename T>
ref<typename std::tuple_element<I, T>::type> get(ref<T> self) {
    return tuple_traits<T>::template get<I>(self);
}

template<Native<Size> I, typename T>
mut_ref<typename std::tuple_element<I, T>::type> get(mut_ref<T> self) {
    return tuple_traits<T>::template get<I>(self);
}
