#include "scriptengine.hpp"

// https://docs.oracle.com/javase/specs/jvms/se17/html/jvms-4.html

#define CONSTANTS(X) \
    X(Utf8, 1, { \
        Utf8String string; \
    }, (self.string)) \
    X(Integer, 3, { \
        UInt bytes; \
    }, (self.bytes)) \
    X(Float, 4, { \
        UInt bytes; \
    }, (self.bytes)) \
    X(Long, 5, { \
        UInt highBytes; \
        UInt lowBytes; \
    }, (self.highBytes, self.lowBytes)) \
    X(Double, 6, { \
        UInt highBytes; \
        UInt lowBytes; \
    }, (self.highBytes, self.lowBytes)) \
    X(Class, 7, { \
        UShort nameIndex; \
    }, (self.nameIndex)) \
    X(String, 8, { \
        UShort stringIndex; \
    }, (self.stringIndex)) \
    X(Fieldref, 9, { \
        UShort classIndex; \
        UShort nameAndTypeIndex; \
    }, (self.classIndex, self.nameAndTypeIndex)) \
    X(Methodref, 10, { \
        UShort classIndex; \
        UShort nameAndTypeIndex; \
    }, (self.classIndex, self.nameAndTypeIndex)) \
    X(InterfaceMethodref, 11, { \
        UShort classIndex; \
        UShort nameAndTypeIndex; \
    }, (self.classIndex, self.nameAndTypeIndex)) \
    X(NameAndType, 12, { \
        UShort nameIndex; \
        UShort descriptorIndex; \
    }, (self.nameIndex, self.descriptorIndex)) \
    X(MethodHandle, 15, { \
        Byte referenceKind; \
        UShort referenceIndex; \
    }, (self.referenceKind, self.referenceIndex)) \
    X(MethodType, 16, { \
        UShort descriptorIndex; \
    }, (self.descriptorIndex)) \
    X(Dynamic, 17, { \
        UShort bootstrapMethodAttrIndex; \
        UShort nameAndTypeIndex; \
    }, (self.bootstrapMethodAttrIndex, self.nameAndTypeIndex)) \
    X(InvokeDynamic, 18, { \
        UShort bootstrapMethodAttrIndex; \
        UShort nameAndTypeIndex; \
    }, (self.bootstrapMethodAttrIndex, self.nameAndTypeIndex)) \
    X(Module, 19, { \
        UShort nameIndex; \
    }, (self.nameIndex)) \
    X(Package, 20, { \
        UShort nameIndex; \
    }, (self.nameIndex)) \
    /**/

// https://docs.oracle.com/javase/specs/jvms/se17/html/jvms-6.html

#define INSTRUCTIONS(X) \
    X(, athrow, 0xbf, {}, ()) \
    X(, monitorenter, 0xc2, {}, ()) \
    X(, monitorexit, 0xc3, {}, ()) \
    X(, nop, 0x0, {}, ()) \
    X(_, return, 0xb1, {}, ()) \
    X(, wide, 0xc4, { /* TODO */ }, ()) \
    /* class */ \
    X(, anewarray, 0xbd, { UShort index; }, (self.index)) \
    X(, arraylength, 0xbe, {}, ()) \
    X(, checkcast, 0xc0, { UShort index; }, (self.index)) \
    X(, getfield, 0xb4, { UShort index; }, (self.index)) \
    X(, getstatic, 0xb2, { UShort index; }, (self.index)) \
    X(, instanceof, 0xc1, { UShort index; }, (self.index)) \
    X(, multianewarray, 0xc5, { UShort index; Byte dimensions; }, (self.index, self.dimensions)) \
    X(_, new, 0xbb, { UShort index; }, (self.index)) \
    X(, newarray, 0xbc, { Byte atype; }, (self.atype)) \
    X(, putfield, 0xb5, { UShort index; }, (self.index)) \
    X(, putstatic, 0xb3, { UShort index; }, (self.index)) \
    /* invoke */ \
    X(invoke, dynamic, 0xba, { UShort index; UShort _pad; }, (self.index, self._pad)) \
    X(invoke, interface, 0xb9, { UShort index; Byte _count; Byte _pad; }, (self.index, self._count, self._pad)) \
    X(invoke, special, 0xb7, { UShort index; }, (self.index)) \
    X(invoke, static, 0xb8, { UShort index; }, (self.index)) \
    X(invoke, virtual, 0xb6, { UShort index; }, (self.index)) \
    /* stack */ \
    X(, dup, 0x59, {}, ()) \
    X(, dup2, 0x5c, {}, ()) \
    X(, dup2_x1, 0x5d, {}, ()) \
    X(, dup2_x2, 0x5e, {}, ()) \
    X(, dup_x1, 0x5a, {}, ()) \
    X(, dup_x2, 0x5b, {}, ()) \
    X(, ldc, 0x12, { Byte index; }, (self.index)) \
    X(, ldc2_w, 0x14, { UShort index; }, (self.index)) \
    X(, ldc_w, 0x13, { UShort index; }, (self.index)) \
    X(, pop, 0x57, {}, ()) \
    X(, swap, 0x5f, {}, ()) \
    /* branch */ \
    X(_, goto, 0xa7, { Short branch; }, (self.branch)) \
    X(, goto_w, 0xc8, { Int branch; }, (self.branch)) \
    X(, if_acmpeq, 0xa5, { Short branch; }, (self.branch)) \
    X(, if_acmpne, 0xa6, { Short branch; }, (self.branch)) \
    X(, if_icmpeq, 0x9f, { Short branch; }, (self.branch)) \
    X(, if_icmpge, 0xa2, { Short branch; }, (self.branch)) \
    X(, if_icmpgt, 0xa3, { Short branch; }, (self.branch)) \
    X(, if_icmple, 0xa4, { Short branch; }, (self.branch)) \
    X(, if_icmplt, 0xa1, { Short branch; }, (self.branch)) \
    X(, if_icmpne, 0xa0, { Short branch; }, (self.branch)) \
    X(, ifeq, 0x99, { Short branch; }, (self.branch)) \
    X(, ifge, 0x9c, { Short branch; }, (self.branch)) \
    X(, ifgt, 0x9d, { Short branch; }, (self.branch)) \
    X(, ifle, 0x9e, { Short branch; }, (self.branch)) \
    X(, iflt, 0x9b, { Short branch; }, (self.branch)) \
    X(, ifne, 0x9a, { Short branch; }, (self.branch)) \
    X(, ifnonnull, 0xc7, { Short branch; }, (self.branch)) \
    X(, ifnull, 0xc6, { Short branch; }, (self.branch)) \
    X(, jsr, 0xa8, { Short branch; }, (self.branch)) \
    X(, jsr_w, 0xc9, { Int branch; }, (self.branch)) \
    X(, lookupswitch, 0xab, { UInt _default; UInt npairs; /* TODO */ }, ()) \
    X(, ret, 0xa9, { Byte index; }, (self.index)) \
    X(, tableswitch, 0xaa, { UInt _default; UInt low; UInt high; /* TODO */ }, ()) \
    /* reference */ \
    X(a, aload, 0x32, {}, ()) \
    X(a, astore, 0x53, {}, ()) \
    X(a, const_null, 0x1, {}, ()) \
    X(a, load, 0x19, { Byte index; }, (self.index)) \
    X(a, load_0, 0x2a, {}, ()) \
    X(a, load_1, 0x2b, {}, ()) \
    X(a, load_2, 0x2c, {}, ()) \
    X(a, load_3, 0x2d, {}, ()) \
    X(a, return, 0xb0, {}, ()) \
    X(a, store, 0x3a, { Byte index; }, (self.index)) \
    X(a, store_0, 0x4b, {}, ()) \
    X(a, store_1, 0x4c, {}, ()) \
    X(a, store_2, 0x4d, {}, ()) \
    X(a, store_3, 0x4e, {}, ()) \
    /* byte/bool */ \
    X(b, aload, 0x33, {}, ()) \
    X(b, astore, 0x54, {}, ()) \
    X(b, ipush, 0x10, { Int8 value; }, (self.value)) \
    /* char */ \
    X(c, aload, 0x34, {}, ()) \
    X(c, astore, 0x55, {}, ()) \
    /* short */ \
    X(s, aload, 0x35, {}, ()) \
    X(s, astore, 0x56, {}, ()) \
    X(s, ipush, 0x11, { Short value; }, (self.value)) \
    /* int */ \
    X(i, 2b, 0x91, {}, ()) \
    X(i, 2c, 0x92, {}, ()) \
    X(i, 2d, 0x87, {}, ()) \
    X(i, 2f, 0x86, {}, ()) \
    X(i, 2l, 0x85, {}, ()) \
    X(i, 2s, 0x93, {}, ()) \
    X(i, add, 0x60, {}, ()) \
    X(i, aload, 0x2e, {}, ()) \
    X(i, and, 0x7e, {}, ()) \
    X(i, astore, 0x4f, {}, ()) \
    X(i, const_0, 0x3, {}, ()) \
    X(i, const_1, 0x4, {}, ()) \
    X(i, const_2, 0x5, {}, ()) \
    X(i, const_3, 0x6, {}, ()) \
    X(i, const_4, 0x7, {}, ()) \
    X(i, const_5, 0x8, {}, ()) \
    X(i, const_m1, 0x2, {}, ()) \
    X(i, div, 0x6c, {}, ()) \
    X(i, inc, 0x84, { Byte index; Int8 value; }, (self.index, self.value)) \
    X(i, load, 0x15, { Byte index; }, (self.index)) \
    X(i, load_0, 0x1a, {}, ()) \
    X(i, load_1, 0x1b, {}, ()) \
    X(i, load_2, 0x1c, {}, ()) \
    X(i, load_3, 0x1d, {}, ()) \
    X(i, mul, 0x68, {}, ()) \
    X(i, neg, 0x74, {}, ()) \
    X(i, or, 0x80, {}, ()) \
    X(i, rem, 0x70, {}, ()) \
    X(i, return, 0xac, {}, ()) \
    X(i, shl, 0x78, {}, ()) \
    X(i, shr, 0x7a, {}, ()) \
    X(i, store, 0x36, { Byte index; }, (self.index)) \
    X(i, store_0, 0x3b, {}, ()) \
    X(i, store_1, 0x3c, {}, ()) \
    X(i, store_2, 0x3d, {}, ()) \
    X(i, store_3, 0x3e, {}, ()) \
    X(i, sub, 0x64, {}, ()) \
    X(i, ushr, 0x7c, {}, ()) \
    X(i, xor, 0x82, {}, ()) \
    /* long */ \
    X(l, 2d, 0x8a, {}, ()) \
    X(l, 2f, 0x89, {}, ()) \
    X(l, 2i, 0x88, {}, ()) \
    X(l, add, 0x61, {}, ()) \
    X(l, aload, 0x2f, {}, ()) \
    X(l, and, 0x7f, {}, ()) \
    X(l, astore, 0x50, {}, ()) \
    X(l, cmp, 0x94, {}, ()) \
    X(l, const_0, 0x9, {}, ()) \
    X(l, const_1, 0xa, {}, ()) \
    X(l, div, 0x6d, {}, ()) \
    X(l, load, 0x16, { Byte index; }, (self.index)) \
    X(l, load_0, 0x1e, {}, ()) \
    X(l, load_2, 0x1f, {}, ()) \
    X(l, load_3, 0x20, {}, ()) \
    X(l, load_4, 0x21, {}, ()) \
    X(l, mul, 0x69, {}, ()) \
    X(l, neg, 0x75, {}, ()) \
    X(l, or, 0x81, {}, ()) \
    X(l, rem, 0x71, {}, ()) \
    X(l, return, 0xad, {}, ()) \
    X(l, shl, 0x79, {}, ()) \
    X(l, shr, 0x7b, {}, ()) \
    X(l, store, 0x37, { Byte index; }, (self.index)) \
    X(l, store_0, 0x3f, {}, ()) \
    X(l, store_1, 0x40, {}, ()) \
    X(l, store_2, 0x41, {}, ()) \
    X(l, store_3, 0x42, {}, ()) \
    X(l, sub, 0x65, {}, ()) \
    X(l, ushr, 0x7d, {}, ()) \
    X(l, xor, 0x83, {}, ()) \
    /* float */ \
    X(f, 2d, 0x8d, {}, ()) \
    X(f, 2i, 0x8b, {}, ()) \
    X(f, 2l, 0x8c, {}, ()) \
    X(f, add, 0x62, {}, ()) \
    X(f, aload, 0x30, {}, ()) \
    X(f, astore, 0x51, {}, ()) \
    X(f, cmpg, 0x96, {}, ()) \
    X(f, cmpl, 0x95, {}, ()) \
    X(f, const_0, 0xb, {}, ()) \
    X(f, const_1, 0xc, {}, ()) \
    X(f, const_2, 0xd, {}, ()) \
    X(f, div, 0x6e, {}, ()) \
    X(f, load, 0x17, { Byte index; }, (self.index)) \
    X(f, load_0, 0x22, {}, ()) \
    X(f, load_1, 0x23, {}, ()) \
    X(f, load_2, 0x24, {}, ()) \
    X(f, load_3, 0x25, {}, ()) \
    X(f, mul, 0x6a, {}, ()) \
    X(f, neg, 0x76, {}, ()) \
    X(f, rem, 0x72, {}, ()) \
    X(f, return, 0xae, {}, ()) \
    X(f, store, 0x38, { Byte index; }, (self.index)) \
    X(f, store_0, 0x43, {}, ()) \
    X(f, store_1, 0x44, {}, ()) \
    X(f, store_2, 0x45, {}, ()) \
    X(f, store_3, 0x46, {}, ()) \
    X(f, sub, 0x66, {}, ()) \
    /* double */ \
    X(d, 2f, 0x90, {}, ()) \
    X(d, 2i, 0x8e, {}, ()) \
    X(d, 2l, 0x8f, {}, ()) \
    X(d, add, 0x63, {}, ()) \
    X(d, aload, 0x31, {}, ()) \
    X(d, astore, 0x52, {}, ()) \
    X(d, cmpg, 0x98, {}, ()) \
    X(d, cmpl, 0x97, {}, ()) \
    X(d, const_0, 0xe, {}, ()) \
    X(d, const_1, 0xf, {}, ()) \
    X(d, div, 0x6f, {}, ()) \
    X(d, load, 0x18, { Byte index; }, (self.index)) \
    X(d, load_0, 0x26, {}, ()) \
    X(d, load_1, 0x27, {}, ()) \
    X(d, load_2, 0x28, {}, ()) \
    X(d, load_3, 0x29, {}, ()) \
    X(d, mul, 0x6b, {}, ()) \
    X(d, neg, 0x77, {}, ()) \
    X(d, rem, 0x73, {}, ()) \
    X(d, return, 0xaf, {}, ()) \
    X(d, store, 0x39, { Byte index; }, (self.index)) \
    X(d, store_0, 0x47, {}, ()) \
    X(d, store_1, 0x48, {}, ()) \
    X(d, store_2, 0x49, {}, ()) \
    X(d, store_3, 0x4a, {}, ()) \
    X(d, sub, 0x67, {}, ()) \
    /**/

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

    struct Utf8String {
        UShort length;
        Int offset;
    };

    template<>
    struct DecodeTrait<Utf8String> {
        static Utf8String decode(mut_ref<StreamReader> reader) {
            let length = reader.read<UShort>();
            var offset = reader.offset;
            reader.offset = reader.offset + length;
            return Utf8String{
                    .length = length,
                    .offset =offset,
            };
        }
    };

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

    enum class Constant : Native<Byte> {
        Invalid,
#define X(name, _2, _3, _4) name,

        CONSTANTS(X)

#undef X
    };

    enum class JVMConstant : Native<Byte> {
#define X(name, val, _3, _4) name = val,

        CONSTANTS(X)

#undef X
    };

    constexpr Constant internalConstant(JVMConstant c) {
        switch (c) {
#define X(name, _2, _3, _4) case JVMConstant::name: return Constant::name;
            CONSTANTS(X)
#undef X
        }
        return Constant::Invalid;
    }

#define X(name, _2, def, fieldOrder) \
    struct Constant##name##Info def; \
    template <> \
    struct DecodeTrait<Constant##name##Info> { \
        static Constant##name##Info decode(mut_ref<StreamReader> reader) { \
            var self = Constant##name##Info(); \
            Binder{reader}.bind fieldOrder; \
            return self; \
        } \
    };

    CONSTANTS(X)

#undef X

    struct ConstantInfo {
#define X(name, _2, _3, _4) , Constant##name##Info
        using Storage = Variant<Constant CONSTANTS(X)>;
        Variant<Constant CONSTANTS(X)> variant;
#undef X

        explicit ConstantInfo(Storage v) : variant(move(v)) {}

        implicit ConstantInfo(movable<ConstantInfo> other) : variant(move(other.variant)) {}

#define X(name, val, _3, _4) mut_ref<Constant##name##Info> name() { \
        constexpr var t = internalConstant(JVMConstant(val)); \
        return variant.get<Size(0 + Native<Byte>(t))>(); \
    }

        CONSTANTS(X)

#undef X
    };

    template<>
    struct DecodeTrait<ConstantInfo> {
        static ConstantInfo decode(mut_ref<StreamReader> reader) {
            let tag = JVMConstant(Native<Byte>(reader.read<Byte>()));
            switch (tag) {
#define X(name, val, _3, _4) \
                case JVMConstant::name: { \
                    constexpr var i = Size(Native<Byte>(internalConstant(JVMConstant(val)))); \
                    return ConstantInfo(ConstantInfo::Storage::of<i>(reader.read<Constant##name##Info>())); \
                }

                CONSTANTS(X)

#undef X
            }

            die();
        }
    };

    struct AttributeInfo {
        UShort attributeNameIndex;
        Span<Byte> bytes;
    };

    template<>
    struct DecodeTrait<AttributeInfo> {
        static AttributeInfo decode(mut_ref<StreamReader> reader) {
            let attributeNameIndex = reader.read<UShort>();
            let attributeLength = reader.read<UInt>();
            let bytes = Span<Byte>::unsafe(&reader.data.get(reader.offset));
            reader.offset = Int(reader.offset + attributeLength);
            return {
                    .attributeNameIndex = attributeNameIndex,
                    .bytes = bytes,
            };
        }
    };

    struct FieldInfo {
        UShort accessFlags;
        UShort nameIndex;
        UShort descriptorIndex;
        DynArray<AttributeInfo> attributes;
    };

    template<>
    struct DecodeTrait<FieldInfo> {
        static FieldInfo decode(mut_ref<StreamReader> reader) {
            let accessFlags = reader.read<UShort>();
            let nameIndex = reader.read<UShort>();
            let descriptorIndex = reader.read<UShort>();
            var attributes = reader.read<Repeating<AttributeInfo, UShort>>();
            return {
                    .accessFlags = accessFlags,
                    .nameIndex = nameIndex,
                    .descriptorIndex = descriptorIndex,
                    .attributes = move(attributes),
            };
        }
    };

    struct MethodInfo {
        UShort accessFlags;
        UShort nameIndex;
        UShort descriptorIndex;
        DynArray<AttributeInfo> attributes;
    };

    template<>
    struct DecodeTrait<MethodInfo> {
        static MethodInfo decode(mut_ref<StreamReader> reader) {
            let accessFlags = reader.read<UShort>();
            let nameIndex = reader.read<UShort>();
            let descriptorIndex = reader.read<UShort>();
            var attributes = reader.read<Repeating<AttributeInfo, UShort>>();
            return {
                    .accessFlags = accessFlags,
                    .nameIndex = nameIndex,
                    .descriptorIndex = descriptorIndex,
                    .attributes = move(attributes),
            };
        }
    };

    struct Class {
        DynArray<Byte> data;

        UInt magic;
        UShort minorVersion;
        UShort majorVersion;
        DynArray<ConstantInfo> constantPool;
        UShort accessFlags;
        UShort thisClass;
        UShort superClass;
        DynArray<UShort> interfaces;
        DynArray<FieldInfo> fields;
        DynArray<MethodInfo> methods;
        DynArray<AttributeInfo> attributes;
    };

    void ClassHandle::release() {
        var ret = this->handle._value;
        delete ret;
    }

    template<>
    struct DecodeTrait<Class> {
        static Class decode(mut_ref<StreamReader> reader, DynArray<Byte> data) {
            let magic = reader.read<UInt>();
            let minorVersion = reader.read<UShort>();
            let majorVersion = reader.read<UShort>();
            var constantPool = reader.read<Repeating<ConstantInfo, UShort, -1>>();
            let accessFlags = reader.read<UShort>();
            let thisClass = reader.read<UShort>();
            let superClass = reader.read<UShort>();
            var interfaces = reader.read<Repeating<UShort, UShort>>();
            var fields = reader.read<Repeating<FieldInfo, UShort>>();
            var methods = reader.read<Repeating<MethodInfo, UShort>>();
            var attributes = reader.read<Repeating<AttributeInfo, UShort>>();
            return {
                    .data = move(data),
                    .magic = magic,
                    .minorVersion = minorVersion,
                    .majorVersion = majorVersion,
                    .constantPool = move(constantPool),
                    .accessFlags = accessFlags,
                    .thisClass = thisClass,
                    .superClass = superClass,
                    .interfaces = move(interfaces),
                    .fields = move(fields),
                    .methods = move(methods),
                    .attributes = move(attributes),
            };
        }
    };

    enum class Instruction : Native<Byte> {
        Invalid,
#define X(prefix, name, _3, _4, _5) prefix##name,
        INSTRUCTIONS(X)
#undef X
    };

    enum class JVMInstruction : Native<Byte> {
#define X(prefix, name, val, _4, _5) prefix##name = val,
        INSTRUCTIONS(X)
#undef X
    };

    constexpr Instruction internalInstruction(JVMInstruction c) {
        switch (c) {
#define X(prefix, name, _3, _4, _5) case JVMInstruction::prefix##name: return Instruction::prefix##name;
            INSTRUCTIONS(X)
#undef X
        }
        return Instruction::Invalid;
    }

    namespace instruction {
#define X(prefix, name, val, def, _5) struct prefix##name##Info def;
        INSTRUCTIONS(X)
#undef X
    }

#define X(prefix, name, val, def, fieldOrder) \
    template <> \
    struct DecodeTrait<instruction::prefix##name##Info> { \
        static instruction::prefix##name##Info decode(mut_ref<StreamReader> reader) { \
            var self = instruction::prefix##name##Info(); \
            Binder{reader}.bind fieldOrder; \
            return self; \
        } \
    };

    INSTRUCTIONS(X)

#undef X

    struct InstructionInfo {
#define X(prefix, name, _2, _3, _5) , instruction::prefix##name##Info
        using Storage = Variant<Instruction INSTRUCTIONS(X)>;
        Variant<Instruction INSTRUCTIONS(X)> variant;
#undef X

        explicit InstructionInfo(Storage v) : variant(move(v)) {}

        implicit InstructionInfo(movable<InstructionInfo> other) : variant(move(other.variant)) {}
    };

    template<>
    struct DecodeTrait<InstructionInfo> {
        static InstructionInfo decode(mut_ref<StreamReader> reader) {
            let tag = JVMInstruction(Native<Byte>(reader.read<Byte>()));
            switch (tag) {
#define X(prefix, name, val, _4, _5) \
                case JVMInstruction::prefix##name: { \
                    constexpr var i = Size(Native<Byte>(internalInstruction(JVMInstruction(val)))); \
                    return InstructionInfo(InstructionInfo::Storage::of<i>(reader.read<instruction::prefix##name##Info>())); \
                }
                INSTRUCTIONS(X)
#undef X
            }

            die();
        }
    };

    struct CodeAttribute {
        UShort maxStack;
        UShort maxLocals;
        List<InstructionInfo> code;
    };

    template<>
    struct DecodeTrait<CodeAttribute> {
        static CodeAttribute decode(mut_ref<StreamReader> reader) {
            let maxStack = reader.read<UShort>();
            let maxLocals = reader.read<UShort>();
            let codeSize = reader.read<UInt>();
            var offsets = List<Int>();
            var byteMap = DynArray<Int>(Int(codeSize), [](Int) { return 0; });
            var code = List<InstructionInfo>();
            for (var begin = reader.offset, end = Int(reader.offset + codeSize); reader.offset < end;) {
                offsets.add(reader.offset - begin);
                byteMap.set(reader.offset - begin, code.size());
                code.add(move(reader.read<InstructionInfo>()));
            }
            for (var i : Range<Int>::until(Int(0), code.size())) {
                patch(code.get(i), i, offsets.get(i), byteMap);
            }
            return {
                    .maxStack = maxStack,
                    .maxLocals = maxLocals,
                    .code = move(code),
            };
        }

        static void patch(mut_ref<InstructionInfo> instruction, Int index, Int byteIndex, ref<DynArray<Int>> byteMap) {
#define JUMP_INSTRUCTIONS(X) \
    X(_goto) \
    X(goto_w) \
    X(if_acmpeq) \
    X(if_acmpne) \
    X(if_icmpeq) \
    X(if_icmpge) \
    X(if_icmpgt) \
    X(if_icmple) \
    X(if_icmplt) \
    X(if_icmpne) \
    X(ifeq) \
    X(ifge) \
    X(ifgt) \
    X(ifle) \
    X(iflt) \
    X(ifne) \
    X(ifnonnull) \
    X(ifnull) \
    X(jsr) \
    X(jsr_w) \
    /**/
            var tag = instruction.variant.tag();
            switch (tag) {
                default:
                    break;
#define X(id) \
                case Instruction::id: { \
                    var &it = instruction.variant.get<Size(0 + Native<Byte>(Instruction::id))>(); \
                    it.branch = decltype(it.branch)(byteMap.get(byteIndex + Int(it.branch)) - index); \
                    break; \
                }
                JUMP_INSTRUCTIONS(X)
#undef X
            }
        }
    };

    ClassHandle LoadClass(DynArray<Byte> data) {
        var reader = StreamReader{data.asSpan(), 0};
        var ret = reader.read<Class>(move(data));
        return {new(AllocInfo::of<Class>()) Class(move(ret))};
    }

    void LoadCode(MethodHandle handle) {
        let ret = *handle.handle.handle;
        let method = ret.methods.get(handle.index);
        let attribute = method.attributes.get(0);
        var reader = StreamReader{attribute.bytes, 0};
        var code = reader.read<CodeAttribute>();
        (void) code;
    }
}
