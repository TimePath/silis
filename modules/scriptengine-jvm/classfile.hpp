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
        PAD_BEGIN
#define X(name, _2, def, fieldOrder) struct name##Info def;
        CONSTANTS(X)
#undef X
        PAD_END
    }

    struct ConstantInfo {
#define X(name, _2, _3, _4) , constant::name##Info
        Variant<Constant CONSTANTS(X)> variant_;
        using members = Members<&ConstantInfo::variant_>;
        using Storage = decltype(variant_);
#undef X

        explicit constexpr ConstantInfo() : variant_(Storage::empty()) {}

        explicit ConstantInfo(Storage v) : variant_(move(v)) {}

        implicit constexpr ConstantInfo(movable<ConstantInfo> other) noexcept: ConstantInfo() {
            members::swap(*this, other);
        }

        constexpr mut_ref<ConstantInfo> operator_assign(movable<ConstantInfo> other) noexcept {
            members::swap(*this, other);
            return *this;
        }
    };

    struct AttributeInfo {
        Span<Byte> bytes_;
        UShort attributeNameIndex_;
        PAD(6)

        explicit AttributeInfo(UShort attributeNameIndex, Span<Byte> bytes)
                : bytes_(bytes), attributeNameIndex_(attributeNameIndex) {}
    };

    struct FieldInfo {
        UShort accessFlags_;
        UShort nameIndex_;
        UShort descriptorIndex_;
        PAD(2)
        DynArray<AttributeInfo> attributes_;

        explicit FieldInfo(
                UShort accessFlags,
                UShort nameIndex,
                UShort descriptorIndex,
                DynArray<AttributeInfo> attributes
        ) :
                accessFlags_(accessFlags),
                nameIndex_(nameIndex),
                descriptorIndex_(descriptorIndex),
                attributes_(move(attributes)) {}
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
        UShort accessFlags_;
        UShort nameIndex_;
        UShort descriptorIndex_;
        PAD(2)
        DynArray<AttributeInfo> attributes_;

        explicit MethodInfo(
                UShort accessFlags,
                UShort nameIndex,
                UShort descriptorIndex,
                DynArray<AttributeInfo> attributes
        ) :
                accessFlags_(accessFlags),
                nameIndex_(nameIndex),
                descriptorIndex_(descriptorIndex),
                attributes_(move(attributes)) {}
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
        DynArray<Byte> data_;

        UInt magic_;
        UShort minorVersion_;
        UShort majorVersion_;
        ConstantPool constantPool_;
        UShort accessFlags_;
        UShort thisClass_;
        UShort superClass_;
        PAD(2)
        DynArray<UShort> interfaces_;
        DynArray<FieldInfo> fields_;
        DynArray<MethodInfo> methods_;
        DynArray<AttributeInfo> attributes_;

        explicit Class(
                DynArray<Byte> data,
                UInt magic,
                UShort minorVersion,
                UShort majorVersion,
                ConstantPool constantPool,
                UShort accessFlags,
                UShort thisClass,
                UShort superClass,
                DynArray<UShort> interfaces,
                DynArray<FieldInfo> fields,
                DynArray<MethodInfo> methods,
                DynArray<AttributeInfo> attributes
        ) :
                data_(move(data)),
                magic_(magic),
                minorVersion_(minorVersion),
                majorVersion_(majorVersion),
                constantPool_(move(constantPool)),
                accessFlags_(accessFlags),
                thisClass_(thisClass),
                superClass_(superClass),
                interfaces_(move(interfaces)),
                fields_(move(fields)),
                methods_(move(methods)),
                attributes_(move(attributes)) {}
    };
}
