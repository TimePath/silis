#include "../../test/test.hpp"

#include "../../tier0/tier0.hpp"
#include "../../tier1/tier1.hpp"

#include "../../kernel/kernel.hpp"
#include "../scriptengine.hpp"
#include "../descriptor.hpp"

using namespace test;

struct TestClassLoader : scriptengine::jvm::ClassLoader {
    tier2::SlowMap<StringSpan, Optional<scriptengine::jvm::ClassHandle>> classes_;

    ~TestClassLoader() override {
        for (let clsOpt : classes_.values()) {
            if (!clsOpt.hasValue()) {
                continue;
            }
            let cls = clsOpt.value();
            scriptengine::jvm::unload_class(cls);
        }
    }

    Optional<scriptengine::jvm::ClassHandle> get(StringSpan name) override {
        let clsOptOpt = classes_.get(name);
        if (!clsOptOpt.hasValue()) {
            return Optional<scriptengine::jvm::ClassHandle>::empty();
        }
        let clsOpt = clsOptOpt.value();
        if (!clsOpt.hasValue()) {
            return Optional<scriptengine::jvm::ClassHandle>::empty();
        }
        let ret = clsOpt.value();
        return Optional<scriptengine::jvm::ClassHandle>::of(ret);
    }

    void resolve(mut_ref<scriptengine::jvm::VM> vm, StringSpan cls) override;
};

void TestClassLoader::resolve(mut_ref<scriptengine::jvm::VM> vm, StringSpan cls) {
    var cached = classes_.get(cls);
    if (cached.hasValue()) {
        return;
    }
    classes_.set(cls, Optional<scriptengine::jvm::ClassHandle>::empty());
    if (cls == "Hello") {
        var data = file_read("modules/scriptengine-jvm/tests/Hello.class");
        var ret = scriptengine::jvm::load_class_internal(vm, move(data));
        classes_.set(cls, Optional<scriptengine::jvm::ClassHandle>::of(ret));
        return;
    }
    var data = file_read(cstring(format<"modules/scriptengine-jvm/rt/{}.class">(cls)()));
    var ret = scriptengine::jvm::load_class_internal(vm, move(data));
    classes_.set(cls, Optional<scriptengine::jvm::ClassHandle>::of(ret));
}

TEST("load_class") {
    using namespace scriptengine::jvm;

    var classLoader = TestClassLoader();
    var vm = VM{
            .classLoader=classLoader,
    };
    var cls = load_class(vm, "Hello");
    assert(cls.hasValue());
    let mh = find_method(vm, cls.value(), "main");
    load_code(vm, mh);
    printf("load_class\n");
}

void eval(cstring mainClass);

TEST("eval_hello") { eval("Hello"); }

struct UniqueAnyPtr {
    Optional<ptr<void>> ref_;
    ptr<void(ptr<void>)> delete_;

    template<typename T>
    implicit UniqueAnyPtr(ptr<T> self) :
            ref_(Optional<ptr<void>>::of(self)),
            delete_(+[](ptr<void> ref) { delete ptr<T>(ref); }) {}

    implicit UniqueAnyPtr(movable<UniqueAnyPtr> other) :
            ref_(move(other.ref_)),
            delete_(other.delete_) {}

    ~UniqueAnyPtr() {
        if (!ref_.hasValue()) return;
        delete_(ref_.value());
    }
};

void eval(cstring mainClass) {
    using namespace scriptengine::jvm;

    var classLoader = TestClassLoader();
    var vm = VM{
            .classLoader=classLoader,
    };
    var clsOpt = load_class(vm, mainClass);
    assert(clsOpt.hasValue());
    let cls = clsOpt.value();

    struct PrintStream {
        void println(StringSpan line) {
            printf("%.*s\n", Native<Int>(line.data_.size()), cstring(line.data_.data_));
        }
    };

    struct SystemStatics {
        PrintStream out_;
    };

    struct EvaluatorState {
        mut_ref<VM> vm;
        ClassHandle mainClass;
        SystemStatics systemStatics;
        SlowMap<Tuple<StringSpan, StringSpan>, Stack::Value> statics;
        SlowMap<Tuple<ptr<void>, StringSpan>, Stack::Value> members;
    };

    var state = EvaluatorState{
            .vm = vm,
            .mainClass = cls,
            .systemStatics = {
                    .out_ = PrintStream(),
            },
            .statics = SlowMap<Tuple<StringSpan, StringSpan>, Stack::Value>(),
            .members = SlowMap<Tuple<ptr<void>, StringSpan>, Stack::Value>(),
    };

    struct Object {

    };

    struct ArrayObject : DynArray<ptr<void>> {
        using DynArray::DynArray;
    };

    struct MyEvaluator : Evaluator, EvaluatorState {
        List<UniqueAnyPtr> objects;

        explicit MyEvaluator(movable<EvaluatorState> state) : EvaluatorState(move(state)) {}

        void putstatic(StringSpan cls, StringSpan name, Stack::Value value) override {
            if (cls == "java/lang/System" && name == "out") {
                return;
            }
            statics.set(Tuple(cls, name), move(value));
        }

        Stack::Value getstatic(StringSpan cls, StringSpan name) override {
            if (cls == "java/lang/System" && name == "out") {
                return Stack::Value::of<Stack::ValueKind::Reference>(&systemStatics.out_);
            }
            var val = statics.get(Tuple(cls, name));
            assert(val.hasValue());
            return move(val.value());
        }

        void invokestatic(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Frame> frame) override {
            (void) cls;
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = frame.stack.take(argc);
            let mh = find_method(vm, mainClass, name);
            eval(vm, mh, *this, Frame{
                    .parent=Optional<ptr<Frame>>::of(&frame),
                    .locals=move(locals),
            });
        }

        void invokespecial(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Frame> frame) override {
            if (cls == "java/lang/Object" && name == "<init>") {
                return;
            }
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = frame.stack.take(1 + argc);
            let mh = find_method(vm, mainClass, name);
            eval(vm, mh, *this, Frame{
                    .parent=Optional<ptr<Frame>>::of(&frame),
                    .locals=move(locals),
            });
        }

        void invokevirtual(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Frame> frame) override {
            if (cls == "java/io/PrintStream" && name == "println" && signature == "(Ljava/lang/String;)V") {
                var arg = frame.stack.pop().get<Stack::ValueKind::String>();
                var out = ptr<PrintStream>(frame.stack.pop().get<Stack::ValueKind::Reference>());
                out->println(arg);
                return;
            }
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = frame.stack.take(1 + argc);
            let mh = find_method(vm, mainClass, name);
            eval(vm, mh, *this, Frame{
                    .parent=Optional<ptr<Frame>>::of(&frame),
                    .locals=move(locals),
            });
        }

        Stack::Value _new(StringSpan cls) override {
            (void) cls;
            var ret = ptr<Object>(new(AllocInfo::of<Object>()) Object());
            objects.add(ret);
            return Stack::Value::of<Stack::ValueKind::Reference>(ret);
        }

        void putinstance(ptr<void> self, StringSpan name, Stack::Value value) override {
            members.set(Tuple(self, name), move(value));
        }

        Stack::Value getinstance(ptr<void> self, StringSpan name) override {
            var val = members.get(Tuple(self, name));
            assert(val.hasValue());
            return move(val.value());
        }

        Stack::Value _newarray(StringSpan cls, Int count) override {
            (void) cls;
            var arr = ptr<ArrayObject>(new(AllocInfo::of<ArrayObject>()) ArrayObject(count));
            objects.add(arr);
            return Stack::Value::of<Stack::ValueKind::Reference>(arr);
        }

        Stack::Value arraysize(ptr<void> self) override {
            var arr = ptr<ArrayObject>(self);
            return Stack::Value::of<Stack::ValueKind::Int>(arr->size());
        }

        void arrayset(ptr<void> self, Int index, Stack::Value value) override {
            var arr = ptr<ArrayObject>(self);
            arr->set(index, value.get<Stack::ValueKind::Reference>());
        }

        Stack::Value arrayget(ptr<void> self, Int index) override {
            var arr = ptr<ArrayObject>(self);
            return Stack::Value::of<Stack::ValueKind::Reference>(arr->get(index));
        }
    };

    var evaluator = MyEvaluator(move(state));
    let mhStaticInit = find_method(vm, cls, "<clinit>");
    if (mhStaticInit.index_ >= 0) {
        eval(vm, mhStaticInit, evaluator, Frame());
    }
    let mhMain = find_method(vm, cls, "main");
    eval(vm, mhMain, evaluator, Frame());
}
