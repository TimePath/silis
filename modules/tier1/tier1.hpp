#pragma once

#include "../tier0/tier0.hpp"

namespace tier1 {}
using namespace tier1;

// memory
namespace tier1 {
    struct AllocInfo {
        cstring name;
        Size elementSize;
        Size size;

        template<typename T>
        static AllocInfo of() { return {TypeName<T>(), sizeof(T)}; }
    };
}

mut_ptr<void> operator new(Native<Size> count, mut_ptr<void> ptr) noexcept;

mut_ptr<void> operator new[](Native<Size> count, mut_ptr<void> ptr) noexcept;

mut_ptr<void> operator new(Native<Size> count, AllocInfo info);

mut_ptr<void> operator new[](Native<Size> count, AllocInfo info);

// arrays
namespace tier1 {
    template<typename T>
    struct Array : private DisableCopyConstructible {
    private:
        Int _size;
        mut_ptr<T> memory;
    public:
        ~Array() {
            delete[] memory;
        }

        explicit Array(Int size) : DisableCopyConstructible(Unit()),
                                   _size(size), memory(!size ? nullptr : new(AllocInfo::of<T>()) T[size]) {}

        implicit Array(movable<Array> other) : Array(0) {
            *this = move(other);
        }

        void operator=(movable<Array> other) {
            if (&other == this) return;
            this->~Array();
            _size = other._size;
            other._size = 0;
            memory = other.memory;
            other.memory = nullptr;
        };

        template<typename F>
        Array(Int size, F f) : Array(size) {
            for (var i = 0; i < size; ++i) {
                set(i, f(i));
            }
        }

        Int size() const { return _size; }

        Array copy() const {
            return Array(_size, [this](Int i) { return get(i); });
        }

        ref<T> get(Int index) const {
            return memory[index];
        }

        mut_ref<T> get(Int index) {
            return memory[index];
        }

        void set(Int index, T value) {
            memory[index] = move(value);
        }
    };
}

// string
namespace tier1 {
    struct String {
    private:
        Array<Char> chars;
    public:
        String() : String(Array<Char>(0)) {}

        explicit String(Array<Char> chars) : chars(move(chars)) {}

        template<Native<Size> N>
        implicit String(ref<Native<Char>[N]> chars) : String(Array<Char>(Native < Int > (N), [=](Int i) {
            return chars[i];
        })) {}

        explicit String(Array<Byte> bytes) : String(Array<Char>(bytes.size() + 1, [&](Int i) {
            return i < bytes.size() ? (Native<Char>) bytes.get(i) : '\0';
        })) {}

        Int size() const { return chars.size() - 1; }

        String copy() const { return String(chars.copy()); }

        explicit operator cstring() const { return reinterpret_cast<cstring>(&chars.get(0)); }
    };

    inline String operator ""_s(cstring chars, Native<Size> N) {
        return String(Array<Char>((Native<Int>) (N + 1), [=](Int i) {
            return (decltype(N)) i < N ? chars[i] : '\0';
        }));
    };
}
