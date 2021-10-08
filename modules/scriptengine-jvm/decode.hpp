#pragma once

namespace scriptengine::jvm {
    template<typename T>
    struct DecodeTrait;

    struct StreamReader {
        Span<Byte> data;
        Int offset;

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
            var index = reader.offset;
            reader.offset = reader.offset + 1;
            return reader.data.get(index);
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
    template<typename T, typename I, Native<Int> adjust = 0>
    struct Repeating {
    };

    template<typename T, typename I, Native<Int> adjust>
    struct DecodeTrait<Repeating<T, I, adjust>> {
        static DynArray<T> decode(mut_ref<StreamReader> reader) {
            let size = I(reader.read<I>() + adjust);
            var entries = DynArray<T>(Int(size));
            for (let i : Range<I>::until(I(0), size)) {
                entries.set(Int(i), move(reader.read<T>()));
            }
            return entries;
        }
    };
}
