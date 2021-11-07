#include "../../test/test.hpp"

#include "../../tier0/tier0.hpp"
#include "../../tier1/tier1.hpp"

#include "../../kernel/kernel.hpp"
#include "../scriptengine.hpp"
#include "../descriptor.hpp"

using namespace test;

TEST("load_class") {
    using namespace scriptengine::jvm;

    var data = file_read("modules/scriptengine-jvm/tests/Hello.class");
    var cls = load_class(move(data));
    let mh = find_method(cls, "main");
    load_code(mh);
    cls.release();
    printf("load_class\n");
}

void eval(cstring path);

TEST("eval_hello") { eval("modules/scriptengine-jvm/tests/Hello.class"); }

void eval(cstring path) {
    using namespace scriptengine::jvm;

    var data = file_read(path);
    var cls = load_class(move(data));

    struct PrintStream {
        void println(StringSpan line) {
            printf("%.*s\n", Native<Int>(line.data_.size()), cstring(line.data_.data_));
        }
    };

    struct SystemStatics {
        PrintStream out_;
    };

    struct EvaluatorState {
        ClassHandle mainClass;
        SystemStatics systemStatics;
        SlowMap<Tuple<StringSpan, StringSpan>, Stack::Value> statics;
        SlowMap<Tuple<ptr<void>, StringSpan>, Stack::Value> members;
    };

    var state = EvaluatorState{
            .mainClass = cls,
            .systemStatics = {
                    .out_ = PrintStream(),
            },
            .statics = SlowMap<Tuple<StringSpan, StringSpan>, Stack::Value>(),
            .members = SlowMap<Tuple<ptr<void>, StringSpan>, Stack::Value>(),
    };

    struct Object {

    };

    struct MyEvaluator : Evaluator, EvaluatorState {
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
            return move(statics.get(Tuple(cls, name)).value());
        }

        void invokestatic(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Frame> frame) override {
            (void) cls;
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = frame.stack.take(argc);
            let mh = find_method(mainClass, name);
            eval(mh, *this, Frame{
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
            let mh = find_method(mainClass, name);
            eval(mh, *this, Frame{
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
            let mh = find_method(mainClass, name);
            eval(mh, *this, Frame{
                    .parent=Optional<ptr<Frame>>::of(&frame),
                    .locals=move(locals),
            });
        }

        Stack::Value _new(StringSpan cls) override {
            (void) cls;
            return Stack::Value::of<Stack::ValueKind::Reference>(new(AllocInfo::of<Object>()) Object());
        }

        void putinstance(ptr<void> self, StringSpan name, Stack::Value value) override {
            members.set(Tuple(self, name), move(value));
        }

        Stack::Value getinstance(ptr<void> self, StringSpan name) override {
            return move(members.get(Tuple(self, name)).value());
        }

        Stack::Value _newarray(StringSpan cls, Int count) override {
            (void) cls;
            var arr = new(AllocInfo::of<DynArray<ptr<void>>>()) DynArray<ptr<void>>(count);
            return Stack::Value::of<Stack::ValueKind::Reference>(arr);
        }

        Stack::Value arraysize(ptr<void> self) override {
            var arr = ptr<DynArray<ptr<void>>>(self);
            return Stack::Value::of<Stack::ValueKind::Int>(arr->size());
        }

        void arrayset(ptr<void> self, Int index, Stack::Value value) override {
            var arr = ptr<DynArray<ptr<void>>>(self);
            arr->set(index, value.get<Stack::ValueKind::Reference>());
        }

        Stack::Value arrayget(ptr<void> self, Int index) override {
            var arr = ptr<DynArray<ptr<void>>>(self);
            return Stack::Value::of<Stack::ValueKind::Reference>(arr->get(index));
        }
    };

    var evaluator = MyEvaluator(move(state));
    let mhStaticInit = find_method(cls, "<clinit>");
    if (mhStaticInit.index_ >= 0) {
        eval(mhStaticInit, evaluator, Frame());
    }
    let mhMain = find_method(cls, "main");
    eval(mhMain, evaluator, Frame());
    cls.release();
}
