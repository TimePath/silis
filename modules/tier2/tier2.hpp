#pragma once

#include "../tier1/tier1.hpp"

namespace tier2 {}
using namespace tier2;

// intrusive
namespace tier2 {
    template<typename T>
    struct IntrusiveLinks {
        ptr<T> prev;
        ptr<T> next;
    };

    template<typename T, IntrusiveLinks<T> T::*links>
    class IntrusiveList {
        ptr<T> head;
        ptr<T> tail;
    public:
        void add(ptr<T> value) {
            let lValue = &(value->*links);
            let prev = lValue->prev = tail;
            if (let lPrev = !prev ? nullptr : &(prev->*links)) {
                lPrev->next = value;
            } else {
                head = value;
            }
            tail = value;
        }

        void remove(ptr<T> value) {
            let lValue = &(value->*links);
            let prev = lValue->prev;
            let next = lValue->next;
            if (let lPrev = !prev ? nullptr : &(prev->*links)) {
                lPrev->next = next;
            }
            if (let lNext = !next ? nullptr : &(next->*links)) {
                lNext->prev = prev;
            }
            if (value == head) {
                head = next;
            }
            if (value == tail) {
                tail = prev;
            }
        }
    };
}

// list
namespace tier2 {
    namespace detail {
        template<typename T>
        union Unmanaged {
        private:
            alignas(T) Native<Byte> bytes[sizeof(T)];
            T value;
        public:
            ~Unmanaged() {}

            void destroy() {
                value.~T();
            }

            Unmanaged() : bytes() {}

            explicit Unmanaged(T value) : value(move(value)) {}

            implicit Unmanaged(movable<Unmanaged> other) : Unmanaged() {
                *this = move(other);
            }

            void operator=(movable<Unmanaged> other) {
                if (&other == this) return;
                destroy();
                value = move(other.value);
                let fill = Byte(0);
                for (var i = 0; i < (Native<Int>) sizeof(T); ++i) {
                    other.bytes[i] = fill;
                }
            };

            ref<T> get() const { return value; }

            mut_ref<T> get() { return value; }
        };
    }
    template<typename T>
    class List {
        Int _size;
        Array<detail::Unmanaged<T>> memory;

        void realloc(Int capacity) {
            if (memory.size() >= capacity) {
                return;
            }
            memory = Array<detail::Unmanaged<T>>(capacity * 2, [&](Int i) {
                return (i < _size) ? detail::Unmanaged<T>(move(memory.get(i))) : detail::Unmanaged<T>();
            });
        }

    public:
        ~List() {
            for (var i = 0; i < size(); ++i) {
                memory.get(i).destroy();
            }
        }

        List() : _size(0), memory(0) {}

        Int size() const { return _size; }

        ref<T> get(Int index) const {
            if (!(index >= 0 && index < size())) {
                throw nullptr;
            }
            return memory.get(index).get();
        }

        mut_ref<T> get(Int index) {
            if (!(index >= 0 && index < size())) {
                throw nullptr;
            }
            return memory.get(index).get();
        }

        void set(Int index, T value) {
            memory.set(index, detail::Unmanaged<T>(move(value)));
        }

        void add(T value) {
            let index = _size;
            let nextSize = index + 1;
            realloc(nextSize);
            memory.set(index, detail::Unmanaged<T>(move(value)));
            _size = nextSize;
        }
    };
}

// iteration
namespace tier2 {
    template<typename List, typename T>
    struct ListRef {
        List *list;
        Int idx;
    };

    template<typename List, typename T>
    inline Native<Boolean> operator==(ListRef<List, T> &self, ListRef<List, T> &other) {
        return self.list == other.list;
    }

    template<typename List, typename T>
    inline void operator++(ListRef<List, T> &self) {
        self.idx = self.idx + 1;
        if (self.idx >= self.list->size()) {
            self.list = nullptr;
        }
    }

    template<typename List, typename T>
    inline T &operator*(ListRef<List, T> &self) { return self.list->get(self.idx); }

    template<typename T>
    inline ListRef<const List<T>, const T> begin(const List<T> &self) { return {&self, 0}; }

    template<typename T>
    inline ListRef<List<T>, T> begin(List<T> &self) { return {&self, 0}; }

    template<typename T>
    inline ListRef<const List<T>, const T> end(const List<T> &self) { return {}; }

    template<typename T>
    inline ListRef<List<T>, T> end(List<T> &self) { return {}; }
}
