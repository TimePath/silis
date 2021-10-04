#pragma once

#include "../tier1/tier1.hpp"

namespace tier2 {
    using namespace tier1;
}

// intrusive
namespace tier2 {
    template<typename T>
    struct IntrusiveLinks {
        Native<ptr<T>> prev_;
        Native<ptr<T>> next_;
    };

    template<typename T, IntrusiveLinks<T> T::*links>
    struct IntrusiveList {
    private:
        Native<ptr<T>> head_;
        Native<ptr<T>> tail_;
    public:
        void add(mut_ref<T> value) {
            let lValue = &(value.*links);
            let prev = lValue->prev_ = tail_;
            if (let lPrev = !prev ? nullptr : &(prev->*links)) {
                lPrev->next_ = &value;
            } else {
                head_ = &value;
            }
            tail_ = &value;
        }

        void remove(mut_ref<T> value) {
            let lValue = &(value.*links);
            let prev = lValue->prev_;
            let next = lValue->next_;
            if (let lPrev = !prev ? nullptr : &(prev->*links)) {
                lPrev->next_ = next;
            }
            if (let lNext = !next ? nullptr : &(next->*links)) {
                lNext->prev_ = prev;
            }
            if (&value == head_) {
                head_ = next;
            }
            if (&value == tail_) {
                tail_ = prev;
            }
        }
    };
}

// list
namespace tier2 {
    template<typename T>
    struct List {
    private:
        Int size_;
        DynArray<Unmanaged<T>> data_;

        void realloc(Int capacity) {
            if (data_.size() >= capacity) {
                return;
            }
            data_ = DynArray<Unmanaged<T>>(capacity * 2, [&](Int i) {
                return (i < size_) ? Unmanaged<T>(move(data_.get(i))) : Unmanaged<T>();
            });
        }

    public:
        ~List() {
            for (var i : Range<Int>::until(0, size())) {
                data_.get(i).destroy();
            }
        }

        explicit List() : size_(0), data_(0) {}

        implicit List(movable<List> other) : size_(other.size_), data_(move(other.data_)) {
            other.size_ = 0;
        }

        Int size() const { return size_; }

        ref<T> get(Int index) const {
            if (!Range<Int>::until(0, size()).contains(index)) {
                die();
            }
            return data_.get(index).get();
        }

        mut_ref<T> get(Int index) {
            if (!Range<Int>::until(0, size()).contains(index)) {
                die();
            }
            return data_.get(index).get();
        }

        void set(Int index, T value) {
            data_.get(index).destroy();
            data_.set(index, Unmanaged<T>(move(value)));
        }

        void add(T value) {
            let index = size_;
            let nextSize = index + 1;
            realloc(nextSize);
            data_.set(index, Unmanaged<T>(move(value)));
            size_ = nextSize;
        }

        template<typename E>
        struct Iterator {
            ptr<const List> self_;
            Int index_;

            constexpr Boolean hasNext() const { return index_ < self_->size(); }

            constexpr ref<E> get() const { return self_->get(index_); }

            constexpr void next() { index_ = index_ + 1; }

            ENABLE_FOREACH_ITERATOR(Iterator)
        };

        constexpr Iterator<const T> iterator() const { return {this, 0}; }

        constexpr Iterator<T> iterator() { return {this, 0}; }

        ENABLE_FOREACH_ITERABLE()
    };
}
