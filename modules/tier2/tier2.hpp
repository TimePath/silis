#pragma once

#include "../tier1/tier1.hpp"

namespace tier2 {
    using namespace tier1;
}

// list
namespace tier2 {
    template<typename T>
    struct List {
    private:
        Int size_;
        DynArray<Unmanaged<T>> data_;
    public:
        ~List() {
            for (var i : Range<Int>::until(0, size())) {
                data_.get(i).destroy();
            }
        }

        implicit List() : size_(0), data_(0) {}

        implicit List(movable<List> other) : size_(exchange(other.size_, 0)), data_(move(other.data_)) {}

        Int size() const { return size_; }

        void _size(Int size) { size_ = size; }

        ref<T> get(Int index) const {
            assert(Range<Int>::until(0, size()).contains(index));
            return data_.get(index).get();
        }

        mut_ref<T> get(Int index) {
            assert(Range<Int>::until(0, size()).contains(index));
            return data_.get(index).get();
        }

        void set(Int index, T value) {
            data_.get(index).destroy();
            data_.set(index, Unmanaged<T>(move(value)));
        }

        void add(T value) {
            let index = size_;
            let nextSize = index + 1;
            ensure(nextSize);
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

        void ensure(Int capacity) {
            if (data_.size() >= capacity) {
                return;
            }
            data_ = DynArray<Unmanaged<T>>(capacity * 2, [&](Int i) {
                return (i < size_) ? Unmanaged<T>(move(data_.get(i))) : Unmanaged<T>();
            });
        }
    };
}

// map
namespace tier2 {
    template<typename K, typename V>
    struct SlowMap {
    private:
        List<K> keys_;
        List<V> vals_;
    public:
        ref<List<K>> keys() const { return keys_; }

        ref<List<V>> values() const { return vals_; }

        Optional<V> get(ref<K> key) const {
            var opt = _get(key);
            if (opt.hasValue()) {
                var ptr = opt.value();
                var copy = V(*ptr);
                return Optional<V>::of(move(copy));
            }
            return Optional<V>::empty();
        }

        void set(ref<K> key, V val) {
            var opt = _get(key);
            if (!opt.hasValue()) {
                keys_.add(key);
                vals_.add(move(val));
                return;
            }
            var ptr = opt.value();
            new(ptr) V(move(val));
        }

    private:
        template<typename Self, typename R>
        static Optional<ptr<R>> _get(mut_ref<Self> self, ref<K> key);

        Optional<ptr<const V>> _get(ref<K> key) const { return _get<const SlowMap, const V>(*this, key); }

        Optional<ptr<V>> _get(ref<K> key) { return _get<SlowMap, V>(*this, key); }
    };

    template<typename K, typename V>
    template<typename Self, typename R>
    Optional<ptr<R>> SlowMap<K, V>::_get(mut_ref<Self> self, ref<K> key) {
        var i = Int(0);
        for (let it : self.keys_) {
            if (it == key) {
                return Optional<ptr<R>>::of(ptr<R>(&self.vals_.get(i)));
            }
            i = i + 1;
        }
        return Optional<ptr<R>>::empty();
    }
}
