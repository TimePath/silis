#pragma once

#ifdef SCRIPTENGINE_JVM_EXPORTS
#define SCRIPTENGINE_JVM_EXPORT EXPORT_DLLEXPORT
#else
#define SCRIPTENGINE_JVM_EXPORT EXPORT_DLLIMPORT
#endif

#include "../tier2/tier2.hpp"

#include "instructions.hpp"

namespace scriptengine::jvm {
    using namespace tier2;
}

namespace scriptengine::jvm {
    struct VM;

    struct Class;

    struct ClassHandle {
        ptr<Class> handle_;
    };

    struct MethodHandle {
        ClassHandle handle_;
        Int index_;
        PAD(4)

        explicit MethodHandle(ClassHandle handle, Int index)
                : handle_(handle), index_(index) {}
    };

    struct SCRIPTENGINE_JVM_EXPORT ClassLoader {
        virtual ~ClassLoader();

        virtual Optional<scriptengine::jvm::ClassHandle> get(StringSpan name) = 0;

        virtual void load(mut_ref<VM> vm, StringSpan className) = 0;

        virtual void init(mut_ref<VM> vm, scriptengine::jvm::ClassHandle ch) = 0;
    };

    SCRIPTENGINE_JVM_EXPORT
    Optional<ClassHandle> load_class(mut_ref<VM> vm, StringSpan name);

    SCRIPTENGINE_JVM_EXPORT
    ClassHandle define_class(mut_ref<VM> vm, DynArray<Byte> data);

    SCRIPTENGINE_JVM_EXPORT
    void unload_class(ClassHandle ch);

    SCRIPTENGINE_JVM_EXPORT
    Optional<MethodHandle> find_method(mut_ref<VM> vm, ClassHandle ch, StringSpan name);

    struct CodeAttribute {
        UShort maxStack_;
        UShort maxLocals_;
        PAD(4)
        List<InstructionInfo> code_;

        explicit CodeAttribute(
                UShort maxStack,
                UShort maxLocals,
                List<InstructionInfo> code
        ) :
                maxStack_(maxStack),
                maxLocals_(maxLocals),
                code_(move(code)) {}
    };

    SCRIPTENGINE_JVM_EXPORT
    Optional<CodeAttribute> load_code(mut_ref<VM> vm, MethodHandle handle);

    struct Stack {
        enum class ValueKind {
            Invalid,
            Reference,
            Int,
            Long,
        };

        using Value = Variant<ValueKind, ptr<void>, Int, Long>;

        Int sp_ = Int(0);
        PAD(4)
        List<Value> stack_ = List<Value>();

        mut_ref<Value> peek() { return stack_.get(sp_ - 1); }

        void push(Value v) {
            stack_.ensure(sp_ + 1);
            stack_.set(sp_, move(v));
            sp_ = sp_ + 1;
            stack_._size(sp_);
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
        Stack stack = Stack();
        List<Stack::Value> locals;
    };

    struct SCRIPTENGINE_JVM_EXPORT Evaluator {
        virtual ~Evaluator();

        virtual void putstatic(mut_ref<VM> vm, StringSpan className, StringSpan name, Stack::Value) = 0;

        virtual Stack::Value getstatic(mut_ref<VM> vm, StringSpan className, StringSpan name) = 0;

        virtual void
        invokestatic(mut_ref<VM> vm, StringSpan className, StringSpan name, StringSpan signature,
                     mut_ref<Frame> frame) = 0;

        virtual void
        invokespecial(mut_ref<VM> vm, StringSpan className, StringSpan name, StringSpan signature,
                      mut_ref<Frame> frame) = 0;

        virtual void
        invokevirtual(mut_ref<VM> vm, StringSpan className, StringSpan name, StringSpan signature,
                      mut_ref<Frame> frame) = 0;

        virtual Stack::Value _new(mut_ref<VM> vm, StringSpan className) = 0;

        virtual void putinstance(mut_ref<VM> vm, ptr<void> self, StringSpan name, Stack::Value) = 0;

        virtual Stack::Value getinstance(mut_ref<VM> vm, ptr<void> self, StringSpan name) = 0;

        virtual Stack::Value _newarray(mut_ref<VM> vm, StringSpan className, Int count) = 0;

        virtual Stack::Value arraysize(mut_ref<VM> vm, ptr<void> self) = 0;

        virtual void arrayset(mut_ref<VM> vm, ptr<void> self, Int index, Stack::Value value) = 0;

        virtual Stack::Value arrayget(mut_ref<VM> vm, ptr<void> self, Int index) = 0;
    };

    SCRIPTENGINE_JVM_EXPORT
    void eval(mut_ref<VM> vm, MethodHandle handle, Frame frame);

    struct VM {
        mut_ref<ClassLoader> classLoader;
        mut_ref<Evaluator> evaluator;
    };
}
