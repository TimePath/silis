#pragma once

#include "../tier0/tier0.hpp"

namespace tier1 {
    using namespace tier0;
}

// memory
namespace tier1 {
    struct AllocInfo {
        cstring name;
        Size elementSize;
        Size size;

        template<typename T>
        static AllocInfo of() { return {TypeName<T>(), sizeof(T), Size(0)}; }
    };
}

tier0::Native<tier0::ptr<void>> operator new(tier0::Native<tier0::Size> count, tier1::AllocInfo info);

tier0::Native<tier0::ptr<void>> operator new[](tier0::Native<tier0::Size> count, tier1::AllocInfo info);

// array
namespace tier1 {
    template<typename T>
    struct DynArray : private DisableCopyConstructible {
    private:
        Int _size;
        Native<ptr<T>> memory;

        void acquire() {
            if (!_size) {
                memory = nullptr;
                return;
            }
            var m = operator new[](Size(_size) * sizeof(T), AllocInfo::of<T>());
            var m2 = Native<ptr<T>>(m);
            memory = m2;
        }

        void release() {
            if (!_size) {
                return;
            }
            var m2 = memory;
            var m = Native<ptr<void>>(m2);
            memory = nullptr;
            for (var i : Range<Int>::until(Int(0), _size)) {
                m2[_size - 1 - i].~T();
            }
            operator delete[](m);
        }

    public:
        constexpr ~DynArray() {
            release();
        }

        explicit DynArray() : DisableCopyConstructible(Unit()), _size(0) {}

        explicit DynArray(Int size) : DisableCopyConstructible(Unit()), _size(size) {
            acquire();
        }

        implicit constexpr DynArray(movable<DynArray> other) : DisableCopyConstructible(Unit()),
                                                               _size(other._size), memory(other.memory) {
            other._size = 0;
            other.memory = nullptr;
        }

        constexpr mut_ref<DynArray> operator=(movable<DynArray> other) {
            if (&other == this) return *this;
            this->~DynArray();
            _size = other._size;
            other._size = 0;
            memory = other.memory;
            other.memory = nullptr;
            return *this;
        }

        template<typename F>
        [[gnu::always_inline]] constexpr DynArray(Int size, F f) : DynArray(size) {
            for (var i : Range<Int>::until(0, size)) {
                set(i, f(i));
            }
        }

        constexpr Int size() const { return _size; }

        constexpr DynArray copy() const {
            return DynArray(_size, [this](Int i) { return get(i); });
        }

        constexpr ref<T> get(Int index) const {
            return memory[index];
        }

        constexpr mut_ref<T> get(Int index) {
            return memory[index];
        }

        constexpr void set(Int index, T value) {
            new(&memory[index]) T(move(value));
        }
    };
}

// string
namespace tier1 {
    struct String {
    private:
        DynArray<Char> _chars;
    public:
        explicit String() : String(DynArray<Char>(0)) {}

        explicit String(DynArray<Char> chars) : _chars(move(chars)) {}

        template<Native<Size> N>
        implicit constexpr String(ref<Native<Char>[N]> chars) : String(DynArray<Char>(Native<Int>(N), [=](Int i) {
            return chars[i];
        })) {}

        explicit String(DynArray<Byte> bytes) : String(DynArray<Char>(bytes.size() + 1, [&](Int i) {
            return i < bytes.size() ? Native<Char>(bytes.get(i)) : '\0';
        })) {}

        constexpr Int size() const { return _chars.size() - 1; }

        String copy() const { return String(_chars.copy()); }

        explicit operator cstring() const { return cstring(&_chars.get(0)); }
    };

    inline String operator ""_s(cstring chars, Native<Size> N) {
        return String(DynArray<Char>(Native<Int>(N + 1), [=](Int i) {
            return Size(i) < N ? chars[i] : '\0';
        }));
    }
}
