#pragma once

#include "scriptengine.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

#include "instructions.inc"

namespace scriptengine::jvm {
    using namespace tier0;
    using namespace tier1;
}

namespace scriptengine::jvm {
    enum class Instruction : Native<Byte> {
        Invalid,
#define X(prefix, name, _3, _4, _5) prefix##name,
        INSTRUCTIONS(X)
#undef X
    };

    namespace instruction {
        PAD_BEGIN
#define X(prefix, name, val, def, _5) struct prefix##name##Info def;
        INSTRUCTIONS(X)
#undef X
        PAD_END
    }

    struct InstructionInfo {
#define X(prefix, name, _2, _3, _5) , instruction::prefix##name##Info
        Variant<Instruction INSTRUCTIONS(X)> variant_;
        using members = Members<&InstructionInfo::variant_>;
        using Storage = decltype(variant_);
#undef X

        explicit constexpr InstructionInfo() : variant_(Storage::empty()) {}

        explicit InstructionInfo(Storage v) : variant_(move(v)) {}

        implicit constexpr InstructionInfo(movable<InstructionInfo> other) : InstructionInfo() {
            members::swap(*this, other);
        }
    };
}
