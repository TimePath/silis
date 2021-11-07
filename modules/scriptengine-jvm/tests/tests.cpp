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

    enum class DescriptorAtomKind {
        Void,
        Byte,
        Char,
        Double,
        Float,
        Int,
        Long,
        Reference,
        Short,
        Boolean,
    };

    struct DescriptorAtom {
        DescriptorAtomKind kind_;
        StringSpan value_;
        Int begin_;
        Int end_;
    };

    struct Name {
        Int begin_;
        Int end_;
    };

    // https://docs.oracle.com/javase/specs/jvms/se17/html/jvms-4.html#jvms-4.3
    struct DescriptorParser {
        StringSpan input;
        Int idx;
        List<DescriptorAtom> output;

        static List<DescriptorAtom> parseMethod(StringSpan input) {
            var state = DescriptorParser{
                    .input = input,
                    .output = List<DescriptorAtom>()
            };
            state.methodDescriptor();
            return move(state.output);
        }

        [[maybe_unused]]
        static List<DescriptorAtom> parseField(StringSpan input) {
            var state = DescriptorParser{
                    .input = input,
                    .output = List<DescriptorAtom>()
            };
            state.fieldDescriptor();
            return move(state.output);
        }

    private:
        Boolean error() {
            return false;
        }

        Char peek() {
            return Char(input.data_.get(idx));
        }

        void advance() {
            idx = idx + 1;
        }

        Boolean require(Char c) {
            var ret = peek() == c;
            if (ret) {
                advance();
            }
            return ret;
        }

        Boolean className() {
            var n0 = unqualifiedName();
            if (!n0.hasValue()) return false;
            while (true) {
                if (!require('/')) break;
                var n = unqualifiedName();
                if (!n.hasValue()) break;
            }
            return true;
        }

        Optional<Name> unqualifiedName() {
            var begin = idx;
            while (true) {
                var c = peek();
                if (c == Char('.')) break;
                if (c == Char(';')) break;
                if (c == Char('[')) break;
                if (c == Char('/')) break;
                advance();
            }
            var end = idx;
            if (end == begin) {
                return Optional<Name>::empty();
            }
            return Optional<Name>::of({begin, end});
        }

        void fieldDescriptor() {
            fieldType();
        }

        Boolean fieldType() {
            return baseType() || objectType() || arrayType();
        }

        Boolean baseType() {
            var begin = idx;
            if (require('B')) {
                output.add({DescriptorAtomKind::Byte, input, begin, idx});
                return true;
            }
            if (require('C')) {
                output.add({DescriptorAtomKind::Char, input, begin, idx});
                return true;
            }
            if (require('D')) {
                output.add({DescriptorAtomKind::Double, input, begin, idx});
                return true;
            }
            if (require('F')) {
                output.add({DescriptorAtomKind::Float, input, begin, idx});
                return true;
            }
            if (require('I')) {
                output.add({DescriptorAtomKind::Int, input, begin, idx});
                return true;
            }
            if (require('J')) {
                output.add({DescriptorAtomKind::Long, input, begin, idx});
                return true;
            }
            if (require('S')) {
                output.add({DescriptorAtomKind::Short, input, begin, idx});
                return true;
            }
            if (require('Z')) {
                output.add({DescriptorAtomKind::Boolean, input, begin, idx});
                return true;
            }
            return false;
        }

        Boolean objectType() {
            if (!require('L')) return false;
            var begin = idx;
            if (!className()) return error();
            output.add({DescriptorAtomKind::Reference, input, begin, idx});
            if (!require(';')) return error();
            return true;
        }

        Boolean arrayType() {
            if (!require('[')) return false;
            if (!componentType()) return error();
            return true;
        }

        Boolean componentType() {
            return fieldType();
        }

        Boolean methodDescriptor() {
            if (!require('(')) return false;
            while (parameterDescriptor());
            if (!require(')')) return error();
            if (!returnDescriptor()) return error();
            return true;
        }

        Boolean parameterDescriptor() {
            return fieldType();
        }

        Boolean returnDescriptor() {
            return fieldType() || voidDescriptor();
        }

        Boolean voidDescriptor() {
            var begin = idx;
            if (!require('V')) return false;
            output.add({DescriptorAtomKind::Void, input, begin, idx});
            return true;
        }
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

        void invokestatic(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Stack> stack) override {
            (void) cls;
            (void) signature;
            (void) stack;
            let mh = find_method(mainClass, name);
            eval(mh, *this);
        }

        void invokevirtual(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Stack> stack) override {
            if (cls == "java/io/PrintStream" && name == "println" && signature == "(Ljava/lang/String;)V") {
                var arg = stack.pop().get<Stack::ValueKind::String>();
                var out = ptr<PrintStream>(stack.pop().get<Stack::ValueKind::Reference>());
                out->println(arg);
                return;
            }
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = List<Stack::Value>();
            locals.add(stack.pop());
            for (var i : Range<Int>::until(0, argc)) {
                locals.add(stack.pop());
            }
            let mh = find_method(mainClass, name);
            eval(mh, *this, move(locals));
        }

        void invokespecial(StringSpan cls, StringSpan name, StringSpan signature, mut_ref<Stack> stack) override {
            if (cls == "java/lang/Object" && name == "<init>") {
                return;
            }
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = List<Stack::Value>();
            locals.add(stack.pop());
            for (var i : Range<Int>::until(0, argc)) {
                locals.add(stack.pop());
            }
            let mh = find_method(mainClass, name);
            eval(mh, *this, move(locals));
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
        eval(mhStaticInit, evaluator);
    }
    let mhMain = find_method(cls, "main");
    eval(mhMain, evaluator);
    cls.release();
}
