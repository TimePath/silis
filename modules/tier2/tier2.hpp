#pragma once

#ifdef LIBTIER2_EXPORTS
#define LIBTIER2_EXPORT EXPORT_DLLEXPORT
#else
#define LIBTIER2_EXPORT EXPORT_DLLIMPORT
#endif

#include "../tier1/tier1.hpp"

namespace tier2 {
    LIBTIER2_EXPORT void dummy();

    using namespace tier1;
}

// list
namespace tier2 {
    template<typename T>
    struct List {
    private:
        Int size_;
        PAD(4)
        DynArray<Unmanaged<T>> data_;
        using members = Members<&List::size_, &List::data_>;
    public:
        ~List() {
            for (var i : Range<Int>::until(0, size())) {
                data_.get(i).destroy();
            }
        }

        implicit constexpr List() : size_(0), data_() {}

        implicit constexpr List(movable<List> other) noexcept: List() {
            members::swap(*this, other);
        }

        constexpr mut_ref<List> operator_assign(movable<List> other) noexcept {
            members::swap(*this, other);
            return *this;
        }

        Int size() const { return size_; }

        void _size(Int size) { size_ = size; }

        ref<T> get(Int index) const {
            return data_.get(index).get();
        }

        mut_ref<T> get(Int index) {
            return data_.get(index).get();
        }

        void set(Int index, T value) {
            var &it = data_.get(index);
            it.destroy();
            it.set(move(value));
        }

        constexpr Span<const T> asSpan() const {
            if (size_ == Int(0)) {
                return Span<const T>::empty();
            }
            return Span<const T>::unsafe(ptr<const T>::reinterpret(data_.asSpan().data_), Int(size_));
        }

        constexpr Span<T> asSpan() {
            if (size_ == Int(0)) {
                return Span<T>::empty();
            }
            return Span<T>::unsafe(ptr<T>::reinterpret(data_.asSpan().data_), Int(size_));
        }

        void add(T value) {
            let index = size_;
            let nextSize = index + 1;
            ensure(nextSize);
            data_.set(index, Unmanaged<T>(move(value)));
            size_ = nextSize;
        }

        template<typename E>
        using Iterator = ContiguousIterator<List, E>;

        constexpr Iterator<const T> iterator() const { return Iterator<const T>{this}; }

        constexpr Iterator<T> iterator() { return Iterator<T>{this}; }

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

namespace tier2 {
    template<typename T>
    struct Element {
        T key_;
        Int id_;
        PAD(4)

        explicit Element(T key, Int id)
                : key_(key), id_(id) {}
    };

    enum class Action {
        InsertFirst,
        Replace,
        InsertBefore,
        InsertAfter,
    };

    struct Result {
        Action action_;
        Int index_;
    };

    template<typename T, typename T_cmp>
    Result bsearch(Span<const Element<T>> values, ref<T> key, Int base) {
        var n = values.size();
        if (!n) {
            return {Action::InsertFirst, 0};
        }
        while (true) {
            var half = n / 2;
            var mid = base + half;
            var result = T_cmp()(key, values.get(Int(mid)).key_);
            if (n == Int(1)) {
                switch (result) {
                    case Order::Undefined: {
                        return {Action::Replace, mid};
                    }
                    case Order::Before: {
                        return {Action::InsertBefore, mid};
                    }
                    case Order::After: {
                        return {Action::InsertAfter, mid};
                    }
                }
            }
            switch (result) {
                case Order::Undefined: {
                    return {Action::Replace, mid};
                }
                case Order::Before: {
                    n = n / 2;
                    continue;
                }
                case Order::After: {
                    base = mid;
                    n = n - half;
                    continue;
                }
            }
        }
    }

    template<typename T, typename T_cmp>
    struct Index {
        List<Element<T>> elements_;

        void add(ref<T> key, Int ptr) {
            let constElements = elements_;
            var ret = bsearch<T, T_cmp>(constElements.asSpan(), key, 0);
            switch (ret.action_) {
                case Action::InsertFirst: {
                    elements_.add(Element<T>(key, ptr));
                    break;
                }
                case Action::Replace: {
                    break;
                }
                case Action::InsertBefore: {
                    splice(ret.index_, Element<T>(key, ptr));
                    break;
                }
                case Action::InsertAfter: {
                    splice(ret.index_ + 1, Element<T>(key, ptr));
                    break;
                }
            }
        }

        Optional<Int> find(ref<T> key) const {
            var ret = bsearch<T, T_cmp>(elements_.asSpan(), key, 0);
            if (ret.action_ != Action::Replace) {
                return Optional<Int>::empty();
            }
            let element = elements_.get(Int(ret.index_));
            return Optional<Int>::of(element.id_);
        }

    private:
        void splice(Int index, Element<T> element) {
            let size = elements_.size();
            elements_.ensure(size + 1);
            elements_._size(size + 1);
            if (index < size) {
                for (var i : Range<Int>::until(0, size - index)) {
                    var j = size - (i + 1);
                    elements_.set(j + 1, move(elements_.get(j)));
                }
            }
            elements_.set(Int(index), move(element));
        }
    };
}

// map
namespace tier2 {
    template<typename T>
    struct Comparator {
        Order operator_invoke(ref<T> a, ref<T> b) { return a <=> b; }
    };

    template<typename K, typename V>
    struct SlowMap {
    private:
        List<K> keys_;
        List<V> vals_;
        Index<K, Comparator<K>> index_;
    public:
        implicit SlowMap() {}

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
                var i = keys_.size();
                keys_.add(key);
                vals_.add(move(val));
                index_.add(key, i);
                return;
            }
            *opt.value() = move(val);
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
        var iOpt = self.index_.find(key);
        if (!iOpt.hasValue()) {
            return Optional<ptr<R>>::empty();
        }
        var i = iOpt.value();
        return Optional<ptr<R>>::of(ptr<R>(&self.vals_.get(i)));
    }
}
