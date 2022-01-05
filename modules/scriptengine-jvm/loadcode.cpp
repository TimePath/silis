// SPDX-License-Identifier: AFL-3.0
#include "scriptengine.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

#include "classfile.hpp"
#include "decode.hpp"
#include "instructions.inc"

namespace scriptengine::jvm {
    using namespace tier0;
    using namespace tier1;
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
        enum class JVMInstruction : Native<Byte> {
#define X(prefix, name, val, _4, _5) prefix##name = val,
            INSTRUCTIONS(X)
#undef X
        };

        static constexpr Instruction internalInstruction(JVMInstruction c) {
            switch (c) {
#define X(prefix, name, _3, _4, _5) case JVMInstruction::prefix##name: return Instruction::prefix##name;
                INSTRUCTIONS(X)
#undef X
            }
            return Instruction::Invalid;
        }

        static InstructionInfo decode(mut_ref<StreamReader> reader) {
            let tag = JVMInstruction(Native<Byte>(reader.read<Byte>()));
            switch (tag) {
#define X(prefix, name, val, _4, _5) \
                case JVMInstruction::prefix##name: { \
                    constexpr var i = internalInstruction(JVMInstruction(val)); \
                    return InstructionInfo(InstructionInfo::Storage::of<i>(reader.read<instruction::prefix##name##Info>())); \
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
            for (var begin = reader.offset_, end = Int(UInt(reader.offset_) + codeSize); reader.offset_ < end;) {
                offsets.add(reader.offset_ - begin);
                byteMap.set(reader.offset_ - begin, code.size());
                code.add(move(reader.read<InstructionInfo>()));
            }
            for (var i : Range<Int>::until(Int(0), code.size())) {
                patch(code.get(i), i, offsets.get(i), byteMap);
            }
            return CodeAttribute(
                    /*.maxStack_ =*/ maxStack,
                    /*.maxLocals_ =*/ maxLocals,
                    /*.code_ =*/ move(code)
            );
        }

        static void patch(mut_ref<InstructionInfo> instruction, Int index, Int byteIndex, ref<DynArray<Int>> byteMap) {
#pragma clang diagnostic push
#pragma warning(push)
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma warning(disable : 4061)
            switch (instruction.variant_.index()) {
                default:
                    break;
#define X(prefix, name, _3, _4, _5) \
                case Instruction::prefix##name: { \
                    var &it = instruction.variant_.get<Instruction::prefix##name>(); \
                    it.branch = decltype(it.branch)(byteMap.get(byteIndex + Int(it.branch)) - index); \
                    break; \
                }
                INSTRUCTIONS_BRANCH_BASIC(X)
#undef X
            }
#pragma clang diagnostic pop
#pragma warning(pop)
        }
    };
}

namespace scriptengine::jvm {
    Optional<MethodHandle> find_method(mut_ref<VM> vm, ClassHandle ch, StringSpan name) {
        (void) vm;
        let pool = ch.handle_->constantPool_;
        var i = Int(0);
        for (let it : ch.handle_->methods_.asSpan()) {
            let methodName = pool.getName(it.nameIndex_);
            if (methodName == name) {
                let mh = MethodHandle(ch, i);
                return Optional<MethodHandle>::of(mh);
            }
            i = i + 1;
        }
        return Optional<MethodHandle>::empty();
    }

    Optional<CodeAttribute> load_code(mut_ref<VM> vm, MethodHandle handle) {
        (void) vm;
        let ch = handle.handle_;
        let method = ch.handle_->methods_.get(handle.index_);
        if (method.accessFlags_ & (1 << Native<Int>(AccessFlag::Native))) {
            let pool = ch.handle_->constantPool_;
            let className = pool.getClassName(ch.handle_->thisClass_);
            let methodName = pool.getName(method.nameIndex_);
            var it = Tuple(className, methodName);
            if (false) {
            } else if (it == Tuple<StringSpan, StringSpan>("java/lang/Object", "registerNatives")) {
                return Optional<CodeAttribute>::empty();
            } else if (it == Tuple<StringSpan, StringSpan>("java/net/Inet4Address", "init")) {
                return Optional<CodeAttribute>::empty();
            } else {
                die();
            }
        }
        let attribute = method.attributes_.get(0);
        var reader = StreamReader(attribute.bytes_, 0);
        var code = reader.read<CodeAttribute>();
        return Optional<CodeAttribute>::of(move(code));
    }
}
