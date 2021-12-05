#pragma once

#include "../tier0/tier0.hpp"
#include "../alloc/alloc.hpp"

namespace tier1 {
    using namespace tier0;
}

// array
namespace tier1 {
    namespace detail {
        struct tag_uninitialized {
        };
    }

    template<typename T>
    struct DynArray : private DisableCopyConstructible {
        // private:
        Int size_;
        PAD(4)
        Native<ptr<T>> data_;
        using members = Members<&DynArray::size_, &DynArray::data_>;
        using tag_uninitialized = detail::tag_uninitialized;
    public:
        constexpr ~DynArray() { release(); }

        explicit constexpr DynArray() : DisableCopyConstructible(Unit()), size_(0), data_(nullptr) {}

        constexpr DynArray copy() const {
            return DynArray(size_, [this](Int i) { return get(i); });
        }

        implicit constexpr DynArray(movable<DynArray> other) : DynArray() {
            members::swap(*this, other);
        }

        constexpr mut_ref<DynArray> operator_assign(movable<DynArray> other) {
            members::swap(*this, other);
            return *this;
        }

        static constexpr DynArray uninitialized(Int size) {
            var ret = DynArray<Unmanaged<T>>(tag_uninitialized(), size);
            return move(ret);
        }

        explicit constexpr DynArray(tag_uninitialized, Int size) : DisableCopyConstructible(Unit()),
                                                                   size_(size) {
            acquire();
        }

        implicit constexpr DynArray(movable<DynArray<Unmanaged<T>>> other) :
                DisableCopyConstructible(Unit()),
                size_(exchange(other.size_, 0)),
                data_(Native<ptr<T>>(Native<ptr<void>>(exchange(other.data_, nullptr)))) {
            static_assert(sizeof(Unmanaged<T>) == sizeof(T));
        }

        template<typename F>
        [[gnu::always_inline]] constexpr DynArray(Int size, F f) : DynArray(tag_uninitialized(), size) {
            for (var i : Range<Int>::until(0, size)) {
                emplace<T>(&get(i), f(i));
            }
        }

        [[nodiscard]] constexpr Int size() const { return size_; }

        [[nodiscard]] constexpr ref<T> get(Int index) const {
            assert(Range<Int>::until(0, size()).contains(index));
            return data_[index];
        }

        constexpr mut_ref<T> get(Int index) {
            assert(Range<Int>::until(0, size()).contains(index));
            return data_[index];
        }

        constexpr void set(Int index, T value) {
            assert(Range<Int>::until(0, size()).contains(index));
            data_[index] = move(value);
        }

        [[nodiscard]] constexpr Span<const T> asSpan() const { return Span<const T>::unsafe(data_, Int(size_)); }

        constexpr Span<T> asSpan() { return Span<T>::unsafe(data_, Int(size_)); }

    private:
        void acquire() {
            if (!size_) {
                data_ = nullptr;
                return;
            }
            var m = operator_new[](Size(size_) * sizeof(T), AllocInfo::of<T>());
            var m2 = Native<ptr<T>>(m);
            data_ = m2;
        }

        void release() {
            if (!size_) {
                return;
            }
            var m2 = data_;
            var m = Native<ptr<void>>(m2);
            data_ = nullptr;
            for (var i : Range<Int>::until(Int(0), size_)) {
                m2[size_ - 1 - i].~T();
            }
            operator_delete[](m);
        }
    };
}

// string
namespace tier1 {
    struct String {
    private:
        DynArray<Char> data_;
    public:
        explicit String() : String(DynArray<Char>()) {}

        String copy() const { return String(data_.copy()); }

        template<Native<Size> N>
        implicit constexpr String(ref<Native<Char>[N]> chars) : String(DynArray<Char>(Native<Int>(N), [=](Int i) {
            return chars[i];
        })) {}

        explicit String(DynArray<Char> chars) : data_(move(chars)) {}

        explicit String(DynArray<Byte> bytes) : String(DynArray<Char>(bytes.size() + 1, [&](Int i) {
            return i < bytes.size() ? Native<Char>(bytes.get(i)) : '\0';
        })) {}

        constexpr Int size() const { return data_.size() - 1; }

        explicit operator_convert(cstring)() const { return cstring(&data_.get(0)); }
    };

    inline String operator_udl(_s)(cstring chars, Native<Size> N) {
        return String(DynArray<Char>(Native<Int>(N + 1), [=](Int i) {
            return i < Int(N) ? chars[i] : '\0';
        }));
    }
}

namespace tier1 {
    struct FormatWriter {
        mut_ref<DynArray<Char>> chars_;
        Int offset_;
        PAD(4)

        explicit FormatWriter(mut_ref<DynArray<Char>> chars, Int offset)
                : chars_(chars), offset_(offset) {}

        void write(Char c) {
            chars_.set(offset_, c);
            offset_ = offset_ + 1;
        }
    };

    template<typename T>
    struct FormatTraits;

    template<>
    struct FormatTraits<StringSpan> {
        static constexpr Int size(ref<StringSpan> self) { return self.size(); }

        static void write(ref<StringSpan> self, mut_ref<FormatWriter> writer) {
            for (var c : self.data_.iterator()) {
                writer.write(Char(c));
            }
        }
    };

    template<Size n, typename... Ts>
    struct LazyString {
        const Array<cstring, n> strings_;
        const Tuple<Ts...> values_;

        String operator_invoke() const {
            auto calculateSize = [&]<typename T>(Int acc, ref<T> it, Int i) -> Int {
                var s1 = FormatTraits<T>::size(it);
                var s2 = FormatTraits<StringSpan>::size(strings_.get(Int(1 + i)));
                return acc + s1 + s2;
            };
            var size = forEach(values_, calculateSize, FormatTraits<StringSpan>::size(strings_.get(0)));
            var arr = DynArray<Char>::uninitialized(Int(size + 1));
            var writer = FormatWriter{arr, 0};
            FormatTraits<StringSpan>::write(strings_.get(0), writer);
            auto write = [&]<typename T>(Int acc, ref<T> it, Int i) -> Int {
                FormatTraits<T>::write(it, writer);
                FormatTraits<StringSpan>::write(strings_.get(Int(1 + i)), writer);
                return acc;
            };
            forEach(values_, write, Int(0));
            writer.write(Char(0));
            var ret = String(move(arr));
            return ret;
        }
    };

    template<LiteralString str, typename... Ts>
    auto format(Ts... values) {
        constexpr var N = strtok::count<str>();
        static_assert(sizeof...(Ts) == N - 1, "incorrect number of arguments");
        return LazyString<N, Ts...>{strtok::collect<str>(), Tuple{values...}};
    }
}
