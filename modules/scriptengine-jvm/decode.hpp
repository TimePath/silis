#pragma once
// SPDX-License-Identifier: AFL-3.0

#include "scriptengine.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

namespace scriptengine::jvm {
    using namespace tier0;
    using namespace tier1;
}

namespace scriptengine::jvm {
    template<typename T>
    struct DecodeTrait;

    struct StreamReader {
        Span<Byte> data_;
        Int offset_;
        PAD(4)

        explicit StreamReader(Span<Byte> data, Int offset)
                : data_(data), offset_(offset) {}

        template<typename T, typename... Ts>
        auto read(Ts... args) { return DecodeTrait<T>::decode(*this, move(args)...); }
    };

    struct Binder {
        mut_ref<StreamReader> reader;

        template<typename... Ts>
        constexpr void bind(mut_ref<Ts>... args) { ((void) (args = reader.read<Ts>()), ...); }
    };
}

namespace scriptengine::jvm {
    template<>
    struct DecodeTrait<Byte> {
        static Byte decode(mut_ref<StreamReader> reader) {
            var index = reader.offset_;
            reader.offset_ = reader.offset_ + 1;
            return reader.data_.get(index);
        }
    };

    template<>
    struct DecodeTrait<Int8> {
        static Int8 decode(mut_ref<StreamReader> reader) { return Int8(reader.read<Byte>()); }
    };

    template<>
    struct DecodeTrait<UShort> {
        static UShort decode(mut_ref<StreamReader> reader) {
            let a = reader.read<Byte>();
            let b = reader.read<Byte>();
            return UShort((a << 8) | b);
        }
    };

    template<>
    struct DecodeTrait<Short> {
        static Short decode(mut_ref<StreamReader> reader) { return Short(reader.read<UShort>()); }
    };

    template<>
    struct DecodeTrait<UInt> {
        static UInt decode(mut_ref<StreamReader> reader) {
            let a = reader.read<Byte>();
            let b = reader.read<Byte>();
            let c = reader.read<Byte>();
            let d = reader.read<Byte>();
            return UInt((a << 24) | (b << 16) | (c << 8) | d);
        }
    };

    template<>
    struct DecodeTrait<Int> {
        static Int decode(mut_ref<StreamReader> reader) { return Int(reader.read<UInt>()); }
    };
}

namespace scriptengine::jvm {
    template<typename T>
    struct RepeatingNoSkip {
        static Int size(ref<T>) { return 1; }

        static Unmanaged<T> pad() { return Unmanaged<T>(); }
    };

    template<typename T, typename I, Native<Int> adjust = 0, typename Skip = RepeatingNoSkip<T>>
    struct Repeating {
    };

    template<typename T, typename I, Native<Int> adjust, typename Skip>
    struct DecodeTrait<Repeating<T, I, adjust, Skip>> {
        static DynArray<T> decode(mut_ref<StreamReader> reader) {
            var skip = I(0);
            let size = I(reader.read<I>() + adjust);
            var entries = DynArray<Unmanaged<T>>::uninitialized(Int(size));
            for (let i : Range<I>::until(I(0), size)) {
                if (skip > 0) {
                    entries.set(Int(i), Skip::pad());
                } else {
                    var value = reader.read<T>();
                    skip = I(Skip::size(value));
                    entries.set(Int(i), Unmanaged(move(value)));
                }
                skip = I(skip - I(1));
            }
            return move(entries);
        }
    };
}
