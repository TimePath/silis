#pragma once

#include "scriptengine.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

#include "classfile-constants.inc"

namespace scriptengine::jvm {
    using namespace tier0;
    using namespace tier1;
}

namespace scriptengine::jvm {
    struct Utf8String {
        StringSpan value_;

        implicit Utf8String() : value_(StringSpan{Span<const Byte>::unsafe(nullptr, 0)}) {}

        explicit Utf8String(StringSpan string) : value_(string) {}
    };

    enum class Constant : Native<Byte> {
        Invalid,
#define X(name, _2, _3, _4) name,
        CONSTANTS(X)
#undef X
    };

    namespace constant {
#define X(name, _2, def, fieldOrder) struct name##Info def;
        CONSTANTS(X)
#undef X
    }

    struct ConstantInfo {
#define X(name, _2, _3, _4) , constant::name##Info
        Variant<Constant CONSTANTS(X)> variant_;
        using Storage = decltype(variant_);
#undef X

        explicit ConstantInfo(Storage v) : variant_(move(v)) {}

        implicit ConstantInfo(movable<ConstantInfo> other) : variant_(move(other.variant_)) {}
    };

    struct AttributeInfo {
        UShort attributeNameIndex;
        Span<Byte> bytes;
    };

    struct FieldInfo {
        UShort accessFlags;
        UShort nameIndex;
        UShort descriptorIndex;
        DynArray<AttributeInfo> attributes;
    };

    enum class AccessFlag {
        Public,
        Private,
        Protected,
        Static,
        Final,
        Synchronized,
        Bridge,
        Varargs,
        Native,
        Reserved_,
        Abstract,
        Strict,
        Synthetic,
    };

    struct MethodInfo {
        UShort accessFlags;
        UShort nameIndex;
        UShort descriptorIndex;
        DynArray<AttributeInfo> attributes;
    };

    struct ConstantPool {
        DynArray<ConstantInfo> constants_;

        [[nodiscard]] Span<const ConstantInfo> asSpan() const {
            return constants_.asSpan();
        }

        [[nodiscard]] ref<ConstantInfo> getAny(UShort id) const {
            return constants_.get(id - 1);
        }

        [[nodiscard]] StringSpan getName(UShort id) const {
            return getAny(id).variant_.get<scriptengine::jvm::Constant::Utf8>().string.value_;
        }

        [[nodiscard]] StringSpan getClassName(UShort id) const {
            let refClass = getAny(id).variant_.get<scriptengine::jvm::Constant::Class>();
            return getAny(refClass.nameIndex).variant_.get<scriptengine::jvm::Constant::Utf8>().string.value_;
        }
    };

    struct Class {
        DynArray<Byte> data;

        UInt magic;
        UShort minorVersion;
        UShort majorVersion;
        ConstantPool constantPool;
        UShort accessFlags;
        UShort thisClass;
        UShort superClass;
        DynArray<UShort> interfaces;
        DynArray<FieldInfo> fields;
        DynArray<MethodInfo> methods;
        DynArray<AttributeInfo> attributes;
    };
}
