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

namespace scriptengine::jvm {
    template<typename T>
    struct DecodeTrait;

    struct StreamReader {
        mut_ref<DynArray<Byte>> data;
        Int offset;

        template<typename T>
        auto read() { return DecodeTrait<T>::decode(*this); }
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
    struct DecodeTrait<UShort> {
        static UShort decode(mut_ref<StreamReader> reader) {
            let a = reader.read<Byte>();
            let b = reader.read<Byte>();
            return UShort((a << 8) | b);
        }
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

    struct Binder {
        mut_ref<StreamReader> reader;

        template<typename... Ts>
        constexpr void bind(mut_ref<Ts>... args) { ((void) (args = reader.read<Ts>()), ...); }
    };

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
        using Storage = Variant<Unit CONSTANTS(X)>;
        Variant<Unit CONSTANTS(X)> variant;
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

            throw 0;
        }
    };

    struct AttributeInfo {
        UShort attributeNameIndex;
        ptr<Byte> bytes;
    };

    template<>
    struct DecodeTrait<AttributeInfo> {
        static AttributeInfo decode(mut_ref<StreamReader> reader) {
            let attributeNameIndex = reader.read<UShort>();
            let attributeLength = reader.read<UInt>();
            let bytes = ptr(&reader.data.get(reader.offset));
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
        static Class decode(mut_ref<StreamReader> reader) {
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
                    .data = move(reader.data),
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

    ClassHandle LoadClass(DynArray<Byte> data) {
        var reader = StreamReader{data, 0};
        var ret = reader.read<Class>();
        return {new(AllocInfo::of<Class>()) Class(move(ret))};
    }
}
