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

    MethodHandle find_method(ClassHandle cls, StringSpan name);

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
            Int,
        };

        using Value = Variant<ValueKind, ptr<void>, StringSpan, Int>;

        Int sp_ = Int(0);
        List<Value> stack_ = List<Value>();

        Int size() const { return sp_; }

        void push(Value v) {
            stack_.ensure(sp_ + 1);
            stack_.set(sp_, move(v));
            sp_ = sp_ + 1;
            stack_._size(sp_);
        }

        mut_ref<Value> peek() {
            return stack_.get(sp_ - 1);
        }

        Value pop() {
            var ret = move(peek());
            sp_ = sp_ - 1;
            stack_._size(sp_);
            return ret;
        }

        List<Stack::Value> take(Int n) {
            var ret = List<Stack::Value>();
            var base = sp_ - n;
            for (var i : Range<Int>::until(0, n)) {
                ret.add(move(stack_.get(base + i)));
            }
            sp_ = base;
            stack_._size(sp_);
            return ret;
        }
    };

    struct Frame {
        Optional<ptr<Frame>> parent;
        List<Stack::Value> locals;
        Stack stack;
    };

    struct Evaluator {
        virtual ~Evaluator();

        virtual void putstatic(StringSpan cls, StringSpan name, Stack::Value) = 0;

        virtual Stack::Value getstatic(StringSpan cls, StringSpan name) = 0;

        virtual void invokestatic(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Frame> frame) = 0;

        virtual void invokespecial(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Frame> frame) = 0;

        virtual void invokevirtual(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Frame> frame) = 0;

        virtual Stack::Value _new(StringSpan cls) = 0;

        virtual void putinstance(ptr<void> self, StringSpan name, Stack::Value) = 0;

        virtual Stack::Value getinstance(ptr<void> self, StringSpan name) = 0;

        virtual Stack::Value _newarray(StringSpan cls, Int count) = 0;

        virtual Stack::Value arraysize(ptr<void> self) = 0;

        virtual void arrayset(ptr<void> self, Int index, Stack::Value value) = 0;

        virtual Stack::Value arrayget(ptr<void> self, Int index) = 0;
    };

    void eval(MethodHandle handle, mut_ref<Evaluator> evaluator, Frame frame);
}
