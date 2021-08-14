#pragma once

#include "../tier1/tier1.hpp"

namespace tier2 {}
using namespace tier2;

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
            for (var i = 0; i < size(); ++i) {
                memory.get(i).destroy();
            }
        }

        explicit List() : _size(0), memory(0) {}

        Int size() const { return _size; }

        ref<T> get(Int index) const {
            if (!(index >= 0 and index < size())) {
                die();
            }
            return memory.get(index).get();
        }

        mut_ref<T> get(Int index) {
            if (!(index >= 0 and index < size())) {
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
    inline Boolean operator!=(ref<ListRef<List, T>> self, ref<ListRef<List, T>> other) {
        (void) other;
        return self.list != nullptr;
    }

    template<typename List, typename T>
    inline void operator++(mut_ref<ListRef<List, T>> self) {
        self.idx = self.idx + 1;
        if (self.idx >= self.list->size()) {
            self.list = nullptr;
        }
    }

    template<typename List, typename T>
    inline ref<T> operator*(ref<ListRef<List, T>> self) { return self.list->get(self.idx); }

    template<typename T>
    inline ListRef<const List<T>, const T> begin(ref<List<T>> self) { return {&self, 0}; }

    template<typename T>
    inline ListRef<List<T>, T> begin(mut_ref<List<T>> self) { return {&self, 0}; }

    template<typename T>
    inline ListRef<const List<T>, const T> end(ref<List<T>> self) {
        (void) self;
        return {nullptr, 0};
    }

    template<typename T>
    inline ListRef<List<T>, T> end(mut_ref<List<T>> self) {
        (void) self;
        return {nullptr, 0};
    }
}