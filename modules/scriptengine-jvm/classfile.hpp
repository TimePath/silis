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

        implicit Utf8String() : value_(StringSpan{Span<const Byte>::unsafe(nullptr, Size(0))}) {}

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

    struct MethodInfo {
        UShort accessFlags;
        UShort nameIndex;
        UShort descriptorIndex;
        DynArray<AttributeInfo> attributes;
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
}
