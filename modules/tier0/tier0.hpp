#pragma once

#define var auto
#define let const auto
#define let_ref const auto &

#define implicit

namespace tier0 {}
using namespace tier0;

// meta
namespace tier0 {
    template<typename T>
    struct type_constant {
        using type = T;
    };

    template<typename T>
    struct remove_reference : type_constant<T> {
    };
    template<typename T>
    struct remove_reference<T &> : type_constant<T> {
    };
    template<typename T>
    struct remove_reference<T &&> : type_constant<T> {
    };

    template<typename T, T v>
    struct integral_constant {
        static let value = v;
    };

    using false_type = integral_constant<bool, false>;
    using true_type = integral_constant<bool, true>;

    template<typename T, typename U>
    struct is_same : false_type {
    };
    template<typename T>
    struct is_same<T, T> : true_type {
    };

    template<typename T, unsigned long N>
    struct SizedArray {
        T data[N];

        SizedArray() : data() {}

        implicit SizedArray(T (&data)[N]) : data(data) {}
    };
}

// memory model
namespace tier0 {
    using cstring = char const *;

    template<typename T>
    using ptr = T *;

    template<typename T>
    using mut_ref = T &;

    template<typename T>
    using ref = T const &;

    template<typename T>
    using movable = T &&;

    template<typename T>
    constexpr movable<typename remove_reference<T>::type> move(movable<T> value) noexcept {
        return static_cast<movable<typename remove_reference<T>::type>>(value);
    }
}

// primitives
namespace tier0 {
    namespace detail {
        template<typename T>
        struct is_arithmetic : integral_constant<bool,
                false
                || is_same<bool, T>::value
                || is_same<char, T>::value
                || is_same<short, T>::value
                || is_same<int, T>::value
                || is_same<long, T>::value
                || is_same<float, T>::value
                || is_same<double, T>::value
        > {
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
            requires is_same<T, T2>::value
            implicit constexpr Word(T2 value) noexcept : value(value) {}

            /** Inexact primitive match */
            template<typename U>
            requires (!is_same<T, U>::value) && is_arithmetic<U>::value
            explicit constexpr Word(U value) noexcept : value((T) value) {}

            /** Copy */
            implicit Word(ref<Word> other) noexcept {
                *this = other;
            }

            mut_ref<Word> operator=(ref<Word> other) noexcept {
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
    using Short = WORD(signed short int);
    using UShort = WORD(unsigned short int);
    using Int = WORD(signed int);
    using UInt = WORD(unsigned int);
    using Long = WORD(signed long int);
    using ULong = WORD(unsigned long int);
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

            static constexpr Boolean calculate(ptr<TypeNameFormat> ret) {
                using int_t = decltype(0);
                let_ref needle = "int";
                let_ref haystack = TypeNameRaw<int_t>();
                for (var i = 0;; ++i) {
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
                var ret = TypeNameFormat();
                calculate(&ret);
                return ret;
            }
        };

        constexpr let typeNameFormat = [] {
            static_assert(TypeNameFormat::calculate(nullptr), "Unable to generate type name");
            return TypeNameFormat::calculate();
        }();

        template<typename T>
        constexpr auto TypeName() {
            let_ref raw = TypeNameRaw<T>();
            let n = (sizeof(raw) - 1) - (typeNameFormat.leading + typeNameFormat.trailing);
            var ret = SizedArray<Native<Char>, n + 1>{};
            for (var i = (decltype(n)) 0; i < n; ++i) {
                ret.data[i] = raw[i + typeNameFormat.leading];
            }
            return ret;
        }
    }

    template<typename T>
    cstring TypeName() {
        static let name = detail::TypeName<T>();
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
        DisableCopyConstructible(DisableCopyConstructible const &) = delete;

    protected:
        DisableCopyConstructible(Unit) {}
    };

    class DisableCopyAssignable {
        DisableCopyAssignable &operator=(DisableCopyAssignable const &) = delete;
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
