#include "scriptengine.hpp"

#include "classfile.hpp"
#include "decode.hpp"
#include "instructions.inc"

namespace scriptengine::jvm {
    namespace instruction {
#define X(prefix, name, val, def, _5) struct prefix##name##Info def;
        INSTRUCTIONS(X)
#undef X
    }

    struct InstructionInfo {
#define X(prefix, name, _2, _3, _5) , instruction::prefix##name##Info
        Variant<Instruction INSTRUCTIONS(X)> variant;
        using Storage = decltype(variant);
#undef X

        explicit InstructionInfo(Storage v) : variant(move(v)) {}

        implicit InstructionInfo(movable<InstructionInfo> other) : variant(move(other.variant)) {}
    };

    struct CodeAttribute {
        UShort maxStack;
        UShort maxLocals;
        List<InstructionInfo> code;
    };
}

namespace scriptengine::jvm {
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
}

namespace scriptengine::jvm {
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

    template<>
    struct DecodeTrait<InstructionInfo> {
        static InstructionInfo decode(mut_ref<StreamReader> reader) {
            let tag = JVMInstruction(Native<Byte>(reader.read<Byte>()));
            switch (tag) {
#define X(prefix, name, val, _4, _5) \
                case JVMInstruction::prefix##name: { \
                    constexpr var i = internalInstruction(JVMInstruction(val)); \
                    constexpr var e = Native<Byte>(i); \
                    return InstructionInfo(InstructionInfo::Storage::of<e>(reader.read<instruction::prefix##name##Info>())); \
                }
                INSTRUCTIONS(X)
#undef X
            }

            die();
        }
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
            for (var begin = reader.offset, end = Int(UInt(reader.offset) + codeSize); reader.offset < end;) {
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
            switch (tag) {
#pragma clang diagnostic pop
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
}

namespace scriptengine::jvm {
    void LoadCode(MethodHandle handle) {
        let ret = *handle.handle_.handle_;
        let method = ret.methods.get(handle.index_);
        let attribute = method.attributes.get(0);
        var reader = StreamReader{attribute.bytes, 0};
        var code = reader.read<CodeAttribute>();
        (void) code;
    }
}
