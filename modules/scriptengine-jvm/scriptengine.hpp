#pragma once

#include "../tier2/tier2.hpp"

#include "instructions.hpp"

namespace scriptengine::jvm {
    using namespace tier2;
}

namespace scriptengine::jvm {
    struct Class;

    struct ClassHandle {
        ptr<Class> handle_;

        void release();
    };

    struct MethodHandle {
        ClassHandle handle_;
        Int index_;
    };

    ClassHandle load_class(DynArray<Byte> data);

    struct CodeAttribute {
        UShort maxStack_;
        UShort maxLocals_;
        List<InstructionInfo> code_;
    };

    CodeAttribute load_code(MethodHandle handle);

    struct Stack {
        enum class ValueKind {
            Invalid,
            Reference,
            String,
        };

        using Value = Variant<ValueKind, ptr<void>, StringSpan>;

        Int sp_ = Int(0);
        List<Value> stack_ = List<Value>();

        void push(Value v) {
            stack_.ensure(sp_ + 1);
            stack_.set(sp_, move(v));
            sp_ = sp_ + 1;
            stack_._size(sp_);
        }

        Value pop() {
            sp_ = sp_ - 1;
            var ret = move(stack_.get(sp_));
            stack_._size(sp_);
            return ret;
        }
    };

    struct Evaluator {
        virtual ~Evaluator();

        virtual ptr<void> getstatic(StringSpan cls, StringSpan name) = 0;

        virtual void invokevirtual(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Stack> stack) = 0;
    };

    void eval(ClassHandle handle, mut_ref<Evaluator> evaluator);
}
