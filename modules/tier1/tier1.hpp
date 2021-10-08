#pragma once

#include "../tier0/tier0.hpp"
#include "../alloc/alloc.hpp"

namespace tier1 {
    using namespace tier0;
}

// array
namespace tier1 {
    template<typename T>
    struct DynArray : private DisableCopyConstructible {
    private:
        Int size_;
        Native<ptr<T>> data_;

        void acquire() {
            if (!size_) {
                data_ = nullptr;
                return;
            }
            var m = operator new[](Size(size_) * sizeof(T), AllocInfo::of<T>());
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
            operator delete[](m);
        }

    public:
        constexpr ~DynArray() {
            release();
        }

        explicit DynArray() : DisableCopyConstructible(Unit()), size_(0) {}

        explicit DynArray(Int size) : DisableCopyConstructible(Unit()), size_(size) {
            acquire();
        }

        implicit constexpr DynArray(movable<DynArray> other) : DisableCopyConstructible(Unit()),
                                                               size_(other.size_), data_(other.data_) {
            other.size_ = 0;
            other.data_ = nullptr;
        }

        constexpr mut_ref<DynArray> operator=(movable<DynArray> other) {
            if (&other == this) return *this;
            this->~DynArray();
            size_ = other.size_;
            other.size_ = 0;
            data_ = other.data_;
            other.data_ = nullptr;
            return *this;
        }

        template<typename F>
        [[gnu::always_inline]] constexpr DynArray(Int size, F f) : DynArray(size) {
            for (var i : Range<Int>::until(0, size)) {
                set(i, f(i));
            }
        }

        Span<const T> asSpan() const { return Span<T>::unsafe(data_, Size(size_)); }

        Span<T> asSpan() { return Span<T>::unsafe(data_, Size(size_)); }

        constexpr Int size() const { return size_; }

        constexpr DynArray copy() const {
            return DynArray(size_, [this](Int i) { return get(i); });
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
    };
}

// string
namespace tier1 {
    struct String {
    private:
        DynArray<Char> data_;
    public:
        explicit String() : String(DynArray<Char>(0)) {}

        explicit String(DynArray<Char> chars) : data_(move(chars)) {}

        template<Native<Size> N>
        implicit constexpr String(ref<Native<Char>[N]> chars) : String(DynArray<Char>(Native<Int>(N), [=](Int i) {
            return chars[i];
        })) {}

        explicit String(DynArray<Byte> bytes) : String(DynArray<Char>(bytes.size() + 1, [&](Int i) {
            return i < bytes.size() ? Native<Char>(bytes.get(i)) : '\0';
        })) {}

        constexpr Int size() const { return data_.size() - 1; }

        String copy() const { return String(data_.copy()); }

        explicit operator cstring() const { return cstring(&data_.get(0)); }
    };

    inline String operator ""_s(cstring chars, Native<Size> N) {
        return String(DynArray<Char>(Native<Int>(N + 1), [=](Int i) {
            return Size(i) < N ? chars[i] : '\0';
        }));
    }
}
