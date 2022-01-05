// SPDX-License-Identifier: AFL-3.0
#include "../../test/test.hpp"

#include "../../tier0/tier0.hpp"
#include "../../tier1/tier1.hpp"

#include "../../kernel/kernel.hpp"
#include "../scriptengine.hpp"
#include "../classfile.hpp"
#include "../descriptor.hpp"

using namespace test;
using namespace kernel;

struct TestClassLoader : scriptengine::jvm::ClassLoader {
    tier2::SlowMap<StringSpan, Optional<scriptengine::jvm::ClassHandle>> classes_;
    tier2::SlowMap<StringSpan, Unit> classesInitialized_;

    ~TestClassLoader() override {
        for (let chOpt : classes_.values()) {
            if (!chOpt.hasValue()) {
                continue;
            }
            let ch = chOpt.value();
            scriptengine::jvm::unload_class(ch);
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

    void load(mut_ref<scriptengine::jvm::VM> vm, StringSpan className) override;

    void init(mut_ref<scriptengine::jvm::VM> vm, scriptengine::jvm::ClassHandle ch) override;
};

template<typename F>
void for_each_class(scriptengine::jvm::ClassHandle ch, F block) {
    let pool = ch.handle_->constantPool_;
    let f = [&](ref<scriptengine::jvm::constant::ClassInfo> refClass) {
        let className = pool.getName(refClass.nameIndex);
        if (className.data_.get(0) == Char('[')) {
            return;
        }
        block(className);
    };
    if (ch.handle_->superClass_) {
        f(pool.getAny(ch.handle_->superClass_).variant_.get<scriptengine::jvm::Constant::Class>());
    }
    for (let it : pool.asSpan()) {
        if (it.variant_.index() == scriptengine::jvm::Constant::Class) {
            f(it.variant_.get<scriptengine::jvm::Constant::Class>());
        }
    }
}

static tier2::List<StringSpan>
load_internal(mut_ref<TestClassLoader> self, mut_ref<scriptengine::jvm::VM> vm, Span<const StringSpan> classNames) {
    var dependencies = tier2::List<StringSpan>();
    for (let className : classNames) {
        var data = file_read(
                cstring((className == "Hello" ? format<"modules/scriptengine-jvm/tests/{}.class">(className)
                                              : format<"modules/scriptengine-jvm/rt/{}.class">(className))()));
        var ch = scriptengine::jvm::define_class(vm, move(data));
        self.classes_.set(className, Optional<scriptengine::jvm::ClassHandle>::of(ch));
        for_each_class(ch, [&](StringSpan nextClassName) {
            var cached = self.classes_.get(nextClassName);
            if (cached.hasValue()) {
                return;
            }
            self.classes_.set(nextClassName, Optional<scriptengine::jvm::ClassHandle>::empty());
            dependencies.add(nextClassName);
        });
    }
    return dependencies;
}

void TestClassLoader::load(mut_ref<scriptengine::jvm::VM> vm, StringSpan className) {
    var cached = classes_.get(className);
    if (cached.hasValue()) {
        return;
    }
    const StringSpan classNames[] = {className};
    var next = load_internal(*this, vm, Span<const StringSpan>::unsafe(classNames, 1));
    while (next.size() > 0) {
        let cNext = next;
        next = load_internal(*this, vm, cNext.asSpan());
    }
}

void TestClassLoader::init(mut_ref<scriptengine::jvm::VM> vm, scriptengine::jvm::ClassHandle ch) {
    let pool = ch.handle_->constantPool_;
    let className = pool.getClassName(ch.handle_->thisClass_);
    var cached = classesInitialized_.get(className);
    if (cached.hasValue()) {
        return;
    }
    classesInitialized_.set(className, Unit());

    for_each_class(ch, [&](StringSpan descriptorString) {
        var chNext = vm.classLoader.get(descriptorString);
        assert(chNext.hasValue());
        // init(vm, chNext.value());
    });

    let mhStaticInit = find_method(vm, ch, "<clinit>");
    if (mhStaticInit.hasValue()) {
        eval(vm, mhStaticInit.value(), scriptengine::jvm::Frame());
    }
}

void exec(cstring mainClass);

TEST("exec_hello") { exec("Hello"); }

struct UniqueAnyPtr {
    Optional<ptr<void>> ref_;
    ptr<void(ptr<void>)> delete_;
    using members = Members<&UniqueAnyPtr::ref_, &UniqueAnyPtr::delete_>;

    explicit constexpr UniqueAnyPtr() :
            ref_(Optional<ptr<void>>::empty()),
            delete_(+[](ptr<void>) {}) {}

    template<typename T>
    implicit UniqueAnyPtr(ptr<T> self) :
            ref_(Optional<ptr<void>>::of(self)),
            delete_(+[](ptr<void> ref) { delete ptr<T>(ref); }) {}

    implicit constexpr UniqueAnyPtr(movable<UniqueAnyPtr> other) noexcept: UniqueAnyPtr() {
        members::swap(*this, other);
    }

    ~UniqueAnyPtr() {
        if (!ref_.hasValue()) return;
        delete_(ref_.value());
    }
};

void exec(cstring mainClass) {
    using namespace scriptengine::jvm;

    struct PrintStream {
        void println(StringSpan line) {
            printf("%.*s\n", Native<Int>(line.data_.size()), cstring(line.data_.data_));
        }
    };

    struct Object {
        StringSpan type;
        SlowMap<StringSpan, Stack::Value> fields;
    };

    struct ArrayObject {
        DynArray<Stack::Value> values_;

        explicit ArrayObject(Int size) :
                values_(DynArray<Stack::Value>(size, [](Int) { return Stack::Value::empty(); })) {}

        [[nodiscard]] constexpr Int size() const {
            return values_.size();
        }

        [[nodiscard]] constexpr ref<Stack::Value> get(Int index) const {
            return values_.get(index);
        }

        void set(Int index, Stack::Value value) {
            values_.set(index, move(value));
        }
    };

    struct SystemStatics {
        PrintStream out_;
    };

    struct EvaluatorState {
        SystemStatics systemStatics;
        PAD(7)
        SlowMap<Tuple<StringSpan, StringSpan>, Stack::Value> statics;
    };

    struct MyEvaluator : Evaluator, EvaluatorState {
        List<UniqueAnyPtr> objects;

        explicit MyEvaluator(EvaluatorState state) : EvaluatorState(move(state)) {}

        void putstatic(mut_ref<VM> vm, StringSpan className, StringSpan name, Stack::Value value) override {
            (void) vm;
            if (className == "java/lang/System" && name == "out") {
                return;
            }
            statics.set(Tuple(className, name), move(value));
        }

        Stack::Value getstatic(mut_ref<VM> vm, StringSpan className, StringSpan name) override {
            (void) vm;
            if (className == "java/lang/System" && name == "out") {
                return Stack::Value::of<Stack::ValueKind::Reference>(&systemStatics.out_);
            }
            var val = statics.get(Tuple(className, name));
            assert(val.hasValue());
            return move(val.value());
        }

        void invokestatic(mut_ref<VM> vm, StringSpan className, StringSpan name, StringSpan signature,
                          mut_ref<Frame> frame) override {
            if (className == "java/lang/Class" && name == "getPrimitiveClass") {
                frame.stack.push(Stack::Value::of<Stack::ValueKind::Int>(0));
                return;
            }
            if (className == "java/util/concurrent/atomic/AtomicLong" && name == "VMSupportsCS8") {
                frame.stack.push(Stack::Value::of<Stack::ValueKind::Int>(0));
                return;
            }
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = frame.stack.take(argc);
            let ch = vm.classLoader.get(className);
            assert(ch.hasValue());
            let mh = find_method(vm, ch.value(), name);
            assert(mh.hasValue());
            eval(vm, mh.value(), Frame{
                    .parent=Optional<ptr<Frame>>::of(&frame),
                    .locals=move(locals),
            });
        }

        void invokespecial(mut_ref<VM> vm, StringSpan className, StringSpan name, StringSpan signature,
                           mut_ref<Frame> frame) override {
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = frame.stack.take(1 + argc);
            let ch = vm.classLoader.get(className);
            assert(ch.hasValue());
            let mh = find_method(vm, ch.value(), name);
            assert(mh.hasValue());
            eval(vm, mh.value(), Frame{
                    .parent=Optional<ptr<Frame>>::of(&frame),
                    .locals=move(locals),
            });
        }

        void invokevirtual(mut_ref<VM> vm, StringSpan className, StringSpan name, StringSpan signature,
                           mut_ref<Frame> frame) override {
            if (className == "java/lang/Class" && name == "desiredAssertionStatus") {
                frame.stack.push(Stack::Value::of<Stack::ValueKind::Int>(1));
                return;
            }
            if (className == "java/io/PrintStream" && name == "println" && signature == "(Ljava/lang/String;)V") {
                var arg = frame.stack.pop().get<Stack::ValueKind::Reference>();
                var arr = getinstance(vm, arg, "value").get<Stack::ValueKind::Reference>();
                var out = ptr<PrintStream>(frame.stack.pop().get<Stack::ValueKind::Reference>());
                var size = arraysize(vm, arr).get<Stack::ValueKind::Int>();
                let str = DynArray<Byte>(size, [&](Int i) {
                    return Byte(arrayget(vm, arr, i).get<Stack::ValueKind::Int>());
                });
                out->println(StringSpan(str.asSpan()));
                return;
            }
            var sig = DescriptorParser::parseMethod(signature);
            var argc = sig.size() - 1;
            var locals = frame.stack.take(1 + argc);
            let ch = vm.classLoader.get(className);
            assert(ch.hasValue());
            let mh = find_method(vm, ch.value(), name);
            assert(mh.hasValue());
            eval(vm, mh.value(), Frame{
                    .parent=Optional<ptr<Frame>>::of(&frame),
                    .locals=move(locals),
            });
        }

        Stack::Value _new(mut_ref<VM> vm, StringSpan className) override {
            (void) vm;
            (void) className;
            var ret = ptr<Object>(new(AllocInfo::of<Object>()) Object{
                    .type = className,
                    .fields = {},
            });
            objects.add(ret);
            let fill = [&]() {
                var y = [&](StringSpan _cls, var &rec) mutable -> void {
                    let chOpt = vm.classLoader.get(_cls);
                    let ch = chOpt.value();
                    let pool = ch.handle_->constantPool_;
                    if (ch.handle_->superClass_) {
                        let superClassName = pool.getClassName(ch.handle_->superClass_);
                        rec(superClassName, rec);
                    }
                    for (let field : ch.handle_->fields_.asSpan()) {
                        let refFieldNameString = pool.getName(field.nameIndex_);
                        ret->fields.set(refFieldNameString, Stack::Value::of<Stack::ValueKind::Int>(0));
                    }
                };
                y(className, y);
            };
            fill();
            return Stack::Value::of<Stack::ValueKind::Reference>(ret);
        }

        void putinstance(mut_ref<VM> vm, ptr<void> self, StringSpan name, Stack::Value value) override {
            (void) vm;
            ptr<Object>(self)->fields.set(name, move(value));
        }

        Stack::Value getinstance(mut_ref<VM> vm, ptr<void> self, StringSpan name) override {
            (void) vm;
            var val = ptr<Object>(self)->fields.get(name);
            assert(val.hasValue());
            return move(val.value());
        }

        Stack::Value _newarray(mut_ref<VM> vm, StringSpan className, Int count) override {
            (void) vm;
            (void) className;
            var arr = ptr<ArrayObject>(new(AllocInfo::of<ArrayObject>()) ArrayObject(count));
            objects.add(arr);
            return Stack::Value::of<Stack::ValueKind::Reference>(arr);
        }

        Stack::Value arraysize(mut_ref<VM> vm, ptr<void> self) override {
            (void) vm;
            var arr = ptr<ArrayObject>(self);
            return Stack::Value::of<Stack::ValueKind::Int>(arr->size());
        }

        void arrayset(mut_ref<VM> vm, ptr<void> self, Int index, Stack::Value value) override {
            (void) vm;
            var arr = ptr<ArrayObject>(self);
            arr->set(index, move(value));
        }

        Stack::Value arrayget(mut_ref<VM> vm, ptr<void> self, Int index) override {
            (void) vm;
            var arr = ptr<ArrayObject>(self);
            return arr->get(index).copy();
        }
    };

    var classLoader = TestClassLoader();
    var evaluator = MyEvaluator(
            {
                    .systemStatics = {
                            .out_ = PrintStream(),
                    },
                    .statics = SlowMap<Tuple<StringSpan, StringSpan>, Stack::Value>(),
            }
    );
    var vm = VM{
            .classLoader=classLoader,
            .evaluator=evaluator,
    };

    var chOpt = load_class(vm, mainClass);
    assert(chOpt.hasValue());
    let ch = chOpt.value();

    vm.classLoader.init(vm, ch);

    let mhMain = find_method(vm, ch, "main");
    assert(mhMain.hasValue());
    eval(vm, mhMain.value(), Frame());
}
