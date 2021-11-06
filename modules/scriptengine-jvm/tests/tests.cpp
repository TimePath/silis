#include "../../test/test.hpp"

#include "../../tier0/tier0.hpp"
#include "../../tier1/tier1.hpp"

#include "../../kernel/kernel.hpp"
#include "../scriptengine.hpp"

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
    };

    var state = EvaluatorState{
            .mainClass = cls,
            .systemStatics = {
                    .out_ = PrintStream(),
            }
    };

    struct MyEvaluator : Evaluator, EvaluatorState {
        MyEvaluator(movable<EvaluatorState> state) : EvaluatorState(state) {}

        ptr<void> getstatic(StringSpan cls, StringSpan name) override {
            (void) cls;
            (void) name;
            return &systemStatics.out_;
        }

        void invokestatic(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Stack> stack) override {
            (void) cls;
            (void) signature;
            (void) stack;
            let mh = find_method(mainClass, name);
            eval(mh, *this);
        }

        void invokevirtual(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Stack> stack) override {
            (void) cls;
            (void) name;
            (void) signature;
            var arg = stack.pop().get<Stack::ValueKind::String>();
            var out = ptr<PrintStream>(stack.pop().get<Stack::ValueKind::Reference>());
            out->println(arg);
        }
    };

    var evaluator = MyEvaluator(move(state));
    let mh = find_method(cls, "main");
    eval(mh, evaluator);
    cls.release();
}
