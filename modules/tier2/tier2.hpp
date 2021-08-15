#pragma once

#include "../tier1/tier1.hpp"

namespace tier2 {
    using namespace tier1;
}

// intrusive
namespace tier2 {
    template<typename T>
    struct IntrusiveLinks {
        mut_ptr<T> prev;
        mut_ptr<T> next;
    };

    template<typename T, IntrusiveLinks<T> T::*links>
    struct IntrusiveList {
    private:
        mut_ptr<T> head;
        mut_ptr<T> tail;
    public:
        void add(mut_ref<T> value) {
            let lValue = &(value.*links);
            let prev = lValue->prev = tail;
            if (let lPrev = !prev ? nullptr : &(prev->*links)) {
                lPrev->next = &value;
            } else {
                head = &value;
            }
            tail = &value;
        }

        void remove(mut_ref<T> value) {
            let lValue = &(value.*links);
            let prev = lValue->prev;
            let next = lValue->next;
            if (let lPrev = !prev ? nullptr : &(prev->*links)) {
                lPrev->next = next;
            }
            if (let lNext = !next ? nullptr : &(next->*links)) {
                lNext->prev = prev;
            }
            if (&value == head) {
                head = next;
            }
            if (&value == tail) {
                tail = prev;
            }
        }
    };
}

// list
namespace tier2 {
    template<typename T>
    struct List {
    private:
        Int _size;
        Array<Unmanaged<T>> memory;

        void realloc(Int capacity) {
            if (memory.size() >= capacity) {
                return;
            }
            memory = Array<Unmanaged<T>>(capacity * 2, [&](Int i) {
                return (i < _size) ? Unmanaged<T>(move(memory.get(i))) : Unmanaged<T>();
            });
        }

    public:
        ~List() {
            for (var i : Range<Int>::until(0, size())) {
                memory.get(i).destroy();
            }
        }

        explicit List() : _size(0), memory(0) {}

        Int size() const { return _size; }

        ref<T> get(Int index) const {
            if (!Range<Int>::until(0, size()).contains(index)) {
                die();
            }
            return memory.get(index).get();
        }

        mut_ref<T> get(Int index) {
            if (!Range<Int>::until(0, size()).contains(index)) {
                die();
            }
            return memory.get(index).get();
        }

        void set(Int index, T value) {
            memory.get(index).destroy();
            memory.set(index, Unmanaged<T>(move(value)));
        }

        void add(T value) {
            let index = _size;
            let nextSize = index + 1;
            realloc(nextSize);
            memory.set(index, Unmanaged<T>(move(value)));
            _size = nextSize;
        }

        template<typename E>
        struct Iterator {
            ptr<List> self;
            Int idx;

            constexpr Boolean hasNext() const { return idx < self->size(); }

            constexpr ref<E> get() const { return self->get(idx); }

            constexpr void next() { idx = idx + 1; }

            ENABLE_FOREACH_ITERATOR(Iterator)
        };

        constexpr Iterator<const T> iterator() const { return {this, 0}; }

        constexpr Iterator<T> iterator() { return {this, 0}; }

        ENABLE_FOREACH_ITERABLE(List)
    };
}
