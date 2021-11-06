#include "scriptengine.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

#include "classfile.hpp"
#include "decode.hpp"

namespace scriptengine::jvm {
    using namespace tier0;
    using namespace tier1;
}

namespace scriptengine::jvm {
    template<>
    struct DecodeTrait<Utf8String> {
        static Utf8String decode(mut_ref<StreamReader> reader) {
            let length = reader.read<UShort>();
            var offset = reader.offset;
            let bytes = Span<Byte>::unsafe(&reader.data.get(reader.offset), Size(length));
            reader.offset = reader.offset + length;
            return Utf8String(StringSpan{bytes});
        }
    };

#define X(name, _2, def, fieldOrder) \
    template <> \
    struct DecodeTrait<constant::name##Info> { \
        static constant::name##Info decode(mut_ref<StreamReader> reader) { \
            var self = constant::name##Info(); \
            Binder{reader}.bind fieldOrder; \
            return self; \
        } \
    };

    CONSTANTS(X)

#undef X

    template<>
    struct DecodeTrait<ConstantInfo> {
        enum class JVMConstant : Native<Byte> {
#define X(name, val, _3, _4) name = val,
            CONSTANTS(X)
#undef X
        };

        static constexpr Constant internalConstant(JVMConstant c) {
            switch (c) {
#define X(name, _2, _3, _4) case JVMConstant::name: return Constant::name;
                CONSTANTS(X)
#undef X
            }
            return Constant::Invalid;
        }

        static ConstantInfo decode(mut_ref<StreamReader> reader) {
            let tag = JVMConstant(Native<Byte>(reader.read<Byte>()));
            switch (tag) {
#define X(name, val, _3, _4) \
                case JVMConstant::name: { \
                    constexpr var i = internalConstant(JVMConstant(val)); \
                    return ConstantInfo(ConstantInfo::Storage::of<i>(reader.read<constant::name##Info>())); \
                }

                CONSTANTS(X)

#undef X
            }

            die();
        }
    };

    template<>
    struct DecodeTrait<AttributeInfo> {
        static AttributeInfo decode(mut_ref<StreamReader> reader) {
            let attributeNameIndex = reader.read<UShort>();
            let attributeLength = reader.read<UInt>();
            let bytes = Span<Byte>::unsafe(&reader.data.get(reader.offset), Size(attributeLength));
            reader.offset = Int(UInt(reader.offset) + attributeLength);
            return {
                    .attributeNameIndex = attributeNameIndex,
                    .bytes = bytes,
            };
        }
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
}

namespace scriptengine::jvm {
    ClassHandle load_class(DynArray<Byte> data) {
        var reader = StreamReader{data.asSpan(), 0};
        var ret = reader.read<Class>(move(data));
        return {new(AllocInfo::of<Class>()) Class(move(ret))};
    }

    void ClassHandle::release() {
        var ret = handle_.data_;
        delete ret;
    }
}
