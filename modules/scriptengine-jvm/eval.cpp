#include "scriptengine.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

#include "classfile.hpp"

namespace scriptengine::jvm {
    using namespace tier0;
    using namespace tier1;
}

namespace scriptengine::jvm {
    void eval(mut_ref<VM> vm, MethodHandle mh, Frame frame) {
        var &evaluator = vm.evaluator;
        let ch = mh.handle_;
        let pool = ch.handle_->constantPool_;

        let selfMethod = ch.handle_->methods_.get(mh.index_);
        let selfClassName = pool.getClassName(ch.handle_->thisClass_);
        let selfMethodName = pool.getName(selfMethod.nameIndex_);
        var selfRef = Tuple(selfClassName, selfMethodName);
        (void) selfRef;

        let codeOpt = load_code(vm, mh);
        if (!codeOpt.hasValue()) {
            // native
            return;
        }
        let code = codeOpt.value();
        var &stack = frame.stack;
        var &locals = frame.locals;
        stack.stack_.ensure(Int(code.maxStack_));
        let nLocals = Int(code.maxLocals_);
        locals.ensure(nLocals);
        locals._size(nLocals);
        var run = Boolean(true);
        var unimplemented = [&]() {
            die();
        };
        var pc = Int(0);
        var jump = [&](Int relative) { pc = pc + relative - 1; };
        for (; run; pc = pc + 1) {
            let _instruction = code.code_.get(pc).variant_;
            switch (_instruction.index()) {
                case Instruction::Invalid: {
                    die();
                }
                case Instruction::athrow: {
                    unimplemented();
                    break;
                }
                case Instruction::monitorenter: {
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    (void) self;
                    break;
                }
                case Instruction::monitorexit: {
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    (void) self;
                    break;
                }
                case Instruction::nop: {
                    break;
                }
                case Instruction::_return: {
                    run = false;
                    break;
                }
                case Instruction::wide: {
                    unimplemented();
                    break;
                }
                    // CLASS
                case Instruction::anewarray: {
                    let instruction = _instruction.get<Instruction::anewarray>();
                    var count = stack.pop().get<Stack::ValueKind::Int>();
                    let className = pool.getClassName(instruction.index);
                    stack.push(evaluator._newarray(vm, className, count));
                    break;
                }
                case Instruction::arraylength: {
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(evaluator.arraysize(vm, self));
                    break;
                }
                case Instruction::checkcast: {
                    var objectref = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Reference>(objectref));
                    break;
                }
                case Instruction::getfield: {
                    let instruction = _instruction.get<Instruction::getfield>();
                    let ref = pool.getAny(instruction.index).variant_.get<Constant::Fieldref>();
                    let className = pool.getClassName(ref.classIndex);
                    let refNameAndType = pool.getAny(ref.nameAndTypeIndex).variant_.get<Constant::NameAndType>();
                    let refName = pool.getName(refNameAndType.nameIndex);
                    let refType = pool.getName(refNameAndType.descriptorIndex);
                    (void) className;
                    (void) refType;
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(evaluator.getinstance(vm, self, refName));
                    break;
                }
                case Instruction::getstatic: {
                    let instruction = _instruction.get<Instruction::getstatic>();
                    let ref = pool.getAny(instruction.index).variant_.get<Constant::Fieldref>();
                    let className = pool.getClassName(ref.classIndex);
                    let refNameAndType = pool.getAny(ref.nameAndTypeIndex).variant_.get<Constant::NameAndType>();
                    let refName = pool.getName(refNameAndType.nameIndex);
                    let refType = pool.getName(refNameAndType.descriptorIndex);
                    (void) refType;
                    stack.push(evaluator.getstatic(vm, className, refName));
                    break;
                }
                case Instruction::instanceof: {
                    // let instruction = _instruction.get<Instruction::instanceof>();
                    var objectref = stack.pop().get<Stack::ValueKind::Reference>();
                    (void) objectref;
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(1));
                    break;
                }
                case Instruction::multianewarray: {
                    unimplemented();
                    break;
                }
                case Instruction::_new: {
                    let instruction = _instruction.get<Instruction::_new>();
                    let className = pool.getClassName(instruction.index);
                    stack.push(evaluator._new(vm, className));
                    break;
                }
                case Instruction::newarray: {
                    let instruction = _instruction.get<Instruction::newarray>();
                    var type = instruction.atype;
                    (void) type;
                    var count = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(evaluator._newarray(vm, "<primitive>", count));
                    break;
                }
                case Instruction::putfield: {
                    let instruction = _instruction.get<Instruction::putfield>();
                    let ref = pool.getAny(instruction.index).variant_.get<Constant::Fieldref>();
                    let className = pool.getClassName(ref.classIndex);
                    let refNameAndType = pool.getAny(ref.nameAndTypeIndex).variant_.get<Constant::NameAndType>();
                    let refName = pool.getName(refNameAndType.nameIndex);
                    let refType = pool.getName(refNameAndType.descriptorIndex);
                    (void) className;
                    (void) refType;
                    var value = stack.pop();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    evaluator.putinstance(vm, self, refName, move(value));
                    break;
                }
                case Instruction::putstatic: {
                    let instruction = _instruction.get<Instruction::putstatic>();
                    let ref = pool.getAny(instruction.index).variant_.get<Constant::Fieldref>();
                    let className = pool.getClassName(ref.classIndex);
                    let refNameAndType = pool.getAny(ref.nameAndTypeIndex).variant_.get<Constant::NameAndType>();
                    let refName = pool.getName(refNameAndType.nameIndex);
                    let refType = pool.getName(refNameAndType.descriptorIndex);
                    (void) refType;
                    evaluator.putstatic(vm, className, refName, stack.pop());
                    break;
                }
                    // INVOKE
                case Instruction::invokedynamic: {
                    unimplemented();
                    break;
                }
                case Instruction::invokeinterface: {
                    unimplemented();
                    break;
                }
                case Instruction::invokespecial: {
                    let instruction = _instruction.get<Instruction::invokespecial>();
                    let ref = pool.getAny(instruction.index).variant_.get<Constant::Methodref>();
                    let className = pool.getClassName(ref.classIndex);
                    let refNameAndType = pool.getAny(ref.nameAndTypeIndex).variant_.get<Constant::NameAndType>();
                    let refName = pool.getName(refNameAndType.nameIndex);
                    let refType = pool.getName(refNameAndType.descriptorIndex);
                    evaluator.invokespecial(
                            vm,
                            className, refName, refType,
                            frame
                    );
                    break;
                }
                case Instruction::invokestatic: {
                    let instruction = _instruction.get<Instruction::invokestatic>();
                    let ref = pool.getAny(instruction.index).variant_.get<Constant::Methodref>();
                    let className = pool.getClassName(ref.classIndex);
                    let refNameAndType = pool.getAny(ref.nameAndTypeIndex).variant_.get<Constant::NameAndType>();
                    let refName = pool.getName(refNameAndType.nameIndex);
                    let refType = pool.getName(refNameAndType.descriptorIndex);
                    evaluator.invokestatic(
                            vm,
                            className, refName, refType,
                            frame
                    );
                    break;
                }
                case Instruction::invokevirtual: {
                    let instruction = _instruction.get<Instruction::invokevirtual>();
                    let ref = pool.getAny(instruction.index).variant_.get<Constant::Methodref>();
                    let className = pool.getClassName(ref.classIndex);
                    let refNameAndType = pool.getAny(ref.nameAndTypeIndex).variant_.get<Constant::NameAndType>();
                    let refName = pool.getName(refNameAndType.nameIndex);
                    let refType = pool.getName(refNameAndType.descriptorIndex);
                    evaluator.invokevirtual(
                            vm,
                            className, refName, refType,
                            frame
                    );
                    break;
                }
                    // STACK
                case Instruction::dup: {
                    stack.push(stack.peek().copy());
                    break;
                }
                case Instruction::dup_x1: {
                    var value1 = stack.pop();
                    var value2 = stack.pop();
                    stack.push(value1.copy());
                    stack.push(move(value2));
                    stack.push(move(value1));
                    break;
                }
                case Instruction::dup_x2: {
                    unimplemented();
                    break;
                }
                case Instruction::dup2: {
                    unimplemented();
                    break;
                }
                case Instruction::dup2_x1: {
                    unimplemented();
                    break;
                }
                case Instruction::dup2_x2: {
                    unimplemented();
                    break;
                }
                case Instruction::ldc:
                case Instruction::ldc_w: {
                    var index = _instruction.index() == Instruction::ldc
                                ? UShort(_instruction.get<Instruction::ldc>().index)
                                : UShort(_instruction.get<Instruction::ldc_w>().index);
                    let _constant = pool.getAny(index);
                    var tag = _constant.variant_.index();
                    if (tag == Constant::String) {
                        let constant = _constant.variant_.get<Constant::String>();
                        let str = pool.getName(constant.stringIndex);
                        var arr = evaluator._newarray(vm, "<primitive>", str.size()).get<Stack::ValueKind::Reference>();
                        for (let i : Range<Int>::until(0, str.size())) {
                            evaluator.arrayset(vm, arr, i,
                                               Stack::Value::of<Stack::ValueKind::Int>(Int(str.data_.get(i))));
                        }
                        var ret = evaluator._new(vm, "java/lang/String").get<Stack::ValueKind::Reference>();
                        evaluator.putinstance(vm, ret, "value", Stack::Value::of<Stack::ValueKind::Reference>(arr));
                        stack.push(Stack::Value::of<Stack::ValueKind::Reference>(ret));
                    } else {
                        stack.push(Stack::Value::of<Stack::ValueKind::Int>(0));
                    }
                    break;
                }
                case Instruction::ldc2_w: {
                    unimplemented();
                    break;
                }
                case Instruction::pop: {
                    stack.pop();
                    break;
                }
                case Instruction::swap: {
                    var value1 = stack.pop();
                    var value2 = stack.pop();
                    stack.push(move(value1));
                    stack.push(move(value2));
                    break;
                }
                    // BRANCH
                case Instruction::_goto: {
                    let instruction = _instruction.get<Instruction::_goto>();
                    jump(Int(instruction.branch));
                    break;
                }
                case Instruction::goto_w: {
                    let instruction = _instruction.get<Instruction::goto_w>();
                    jump(Int(instruction.branch));
                    break;
                }
                case Instruction::if_acmpeq: {
                    let instruction = _instruction.get<Instruction::if_acmpeq>();
                    var value2 = stack.pop().get<Stack::ValueKind::Reference>();
                    var value1 = stack.pop().get<Stack::ValueKind::Reference>();
                    if (value1 == value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::if_acmpne: {
                    let instruction = _instruction.get<Instruction::if_acmpne>();
                    var value2 = stack.pop().get<Stack::ValueKind::Reference>();
                    var value1 = stack.pop().get<Stack::ValueKind::Reference>();
                    if (value1 != value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::if_icmpeq: {
                    let instruction = _instruction.get<Instruction::if_icmpeq>();
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    if (value1 == value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::if_icmpge: {
                    let instruction = _instruction.get<Instruction::if_icmpge>();
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    if (value1 >= value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::if_icmpgt: {
                    let instruction = _instruction.get<Instruction::if_icmpgt>();
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    if (value1 > value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::if_icmple: {
                    let instruction = _instruction.get<Instruction::if_icmple>();
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    if (value1 <= value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::if_icmplt: {
                    let instruction = _instruction.get<Instruction::if_icmplt>();
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    if (value1 < value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::if_icmpne: {
                    let instruction = _instruction.get<Instruction::if_icmpne>();
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    if (value1 != value2) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifeq: {
                    let instruction = _instruction.get<Instruction::ifeq>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue == Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifge: {
                    let instruction = _instruction.get<Instruction::ifge>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue >= Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifgt: {
                    let instruction = _instruction.get<Instruction::ifgt>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue > Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifle: {
                    let instruction = _instruction.get<Instruction::ifle>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue <= Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::iflt: {
                    let instruction = _instruction.get<Instruction::iflt>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue < Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifne: {
                    let instruction = _instruction.get<Instruction::ifne>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue != Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifnonnull: {
                    let instruction = _instruction.get<Instruction::ifnonnull>();
                    var ret = Boolean(false);
                    var _value = stack.pop();
                    switch (_value.index()) {
                        case Stack::ValueKind::Reference: {
                            let value = _value.get<Stack::ValueKind::Reference>();
                            ret = value.value() != nullptr;
                            break;
                        }
                        case Stack::ValueKind::Invalid:
                        case Stack::ValueKind::Int:
                        case Stack::ValueKind::Long: {
                            die();
                        }
                    }
                    if (ret) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifnull: {
                    let instruction = _instruction.get<Instruction::ifnull>();
                    var ret = Boolean(false);
                    var _value = stack.pop();
                    switch (_value.index()) {
                        case Stack::ValueKind::Reference: {
                            let value = _value.get<Stack::ValueKind::Reference>();
                            ret = value.value() == nullptr;
                            break;
                        }
                        case Stack::ValueKind::Invalid:
                        case Stack::ValueKind::Int:
                        case Stack::ValueKind::Long: {
                            die();
                        }
                    }
                    if (ret) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::jsr: {
                    unimplemented();
                    break;
                }
                case Instruction::jsr_w: {
                    unimplemented();
                    break;
                }
                case Instruction::lookupswitch: {
                    unimplemented();
                    break;
                }
                case Instruction::ret: {
                    unimplemented();
                    break;
                }
                case Instruction::tableswitch: {
                    unimplemented();
                    break;
                }
                    // REFERENCE
                case Instruction::aaload: {
                    var index = stack.pop().get<Stack::ValueKind::Int>();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(evaluator.arrayget(vm, self, index));
                    break;
                }
                case Instruction::aastore: {
                    var value = stack.pop();
                    var index = stack.pop().get<Stack::ValueKind::Int>();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    evaluator.arrayset(vm, self, index, move(value));
                    break;
                }
                case Instruction::aconst_null: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(0));
                    break;
                }
                case Instruction::aload: {
                    let instruction = _instruction.get<Instruction::aload>();
                    stack.push(locals.get(Int(instruction.index)).copy());
                    break;
                }
                case Instruction::aload_0: {
                    stack.push(locals.get(0).copy());
                    break;
                }
                case Instruction::aload_1: {
                    stack.push(locals.get(1).copy());
                    break;
                }
                case Instruction::aload_2: {
                    stack.push(locals.get(2).copy());
                    break;
                }
                case Instruction::aload_3: {
                    stack.push(locals.get(3).copy());
                    break;
                }
                case Instruction::areturn: {
                    var ret = stack.pop();
                    var &parentFrame = *frame.parent.value();
                    parentFrame.stack.push(move(ret));
                    run = false;
                    break;
                }
                case Instruction::astore: {
                    let instruction = _instruction.get<Instruction::astore>();
                    locals.set(Int(instruction.index), stack.pop());
                    break;
                }
                case Instruction::astore_0: {
                    locals.set(0, stack.pop());
                    break;
                }
                case Instruction::astore_1: {
                    locals.set(1, stack.pop());
                    break;
                }
                case Instruction::astore_2: {
                    locals.set(2, stack.pop());
                    break;
                }
                case Instruction::astore_3: {
                    locals.set(3, stack.pop());
                    break;
                }
                    // BYTE
                case Instruction::baload: {
                    var index = stack.pop().get<Stack::ValueKind::Int>();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(evaluator.arrayget(vm, self, index));
                    break;
                }
                case Instruction::bastore: {
                    unimplemented();
                    break;
                }
                case Instruction::bipush: {
                    let instruction = _instruction.get<Instruction::bipush>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(Int(instruction.value)));
                    break;
                }
                    // CHAR
                case Instruction::caload: {
                    unimplemented();
                    break;
                }
                case Instruction::castore: {
                    unimplemented();
                    break;
                }
                    // SHORT
                case Instruction::saload: {
                    unimplemented();
                    break;
                }
                case Instruction::sastore: {
                    unimplemented();
                    break;
                }
                case Instruction::sipush: {
                    let instruction = _instruction.get<Instruction::sipush>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(Int(instruction.value)));
                    break;
                }
                    // INT
                case Instruction::i2b: {
                    unimplemented();
                    break;
                }
                case Instruction::i2c: {
                    var value = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(Int(UShort(value))));
                    break;
                }
                case Instruction::i2d: {
                    unimplemented();
                    break;
                }
                case Instruction::i2f: {
                    unimplemented();
                    break;
                }
                case Instruction::i2l: {
                    var value = stack.pop().get<Stack::ValueKind::Long>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Long>(Long(value)));
                    break;
                }
                case Instruction::i2s: {
                    var value = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(Int(Short(value))));
                    break;
                }
                case Instruction::iadd: {
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(value1 + value2));
                    break;
                }
                case Instruction::iaload: {
                    var index = stack.pop().get<Stack::ValueKind::Int>();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(evaluator.arrayget(vm, self, index));
                    break;
                }
                case Instruction::iand: {
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(value1 & value2));
                    break;
                }
                case Instruction::iastore: {
                    var value = stack.pop();
                    var index = stack.pop().get<Stack::ValueKind::Int>();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    evaluator.arrayset(vm, self, index, move(value));
                    break;
                }
                case Instruction::iconst_0: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(0));
                    break;
                }
                case Instruction::iconst_1: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(1));
                    break;
                }
                case Instruction::iconst_2: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(2));
                    break;
                }
                case Instruction::iconst_3: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(3));
                    break;
                }
                case Instruction::iconst_4: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(4));
                    break;
                }
                case Instruction::iconst_5: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(5));
                    break;
                }
                case Instruction::iconst_m1: {
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(-1));
                    break;
                }
                case Instruction::idiv: {
                    unimplemented();
                    break;
                }
                case Instruction::iinc: {
                    let instruction = _instruction.get<Instruction::iinc>();
                    locals.set(
                            Int(instruction.index),
                            Stack::Value::of<Stack::ValueKind::Int>(
                                    locals.get(Int(instruction.index)).get<Stack::ValueKind::Int>()
                                    + Int(instruction.value)
                            )
                    );
                    break;
                }
                case Instruction::iload: {
                    let instruction = _instruction.get<Instruction::iload>();
                    stack.push(locals.get(Int(instruction.index)).copy());
                    break;
                }
                case Instruction::iload_0: {
                    stack.push(locals.get(0).copy());
                    break;
                }
                case Instruction::iload_1: {
                    stack.push(locals.get(1).copy());
                    break;
                }
                case Instruction::iload_2: {
                    stack.push(locals.get(2).copy());
                    break;
                }
                case Instruction::iload_3: {
                    stack.push(locals.get(3).copy());
                    break;
                }
                case Instruction::imul: {
                    unimplemented();
                    break;
                }
                case Instruction::ineg: {
                    unimplemented();
                    break;
                }
                case Instruction::ior: {
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(value1 | value2));
                    break;
                }
                case Instruction::irem: {
                    unimplemented();
                    break;
                }
                case Instruction::ireturn: {
                    var ret = stack.pop();
                    var &parentFrame = *frame.parent.value();
                    parentFrame.stack.push(move(ret));
                    run = false;
                    break;
                }
                case Instruction::ishl: {
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(value1 << value2));
                    break;
                }
                case Instruction::ishr: {
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(value1 >> value2));
                    break;
                }
                case Instruction::istore: {
                    let instruction = _instruction.get<Instruction::istore>();
                    locals.set(Int(instruction.index), stack.pop());
                    break;
                }
                case Instruction::istore_0: {
                    locals.set(0, stack.pop());
                    break;
                }
                case Instruction::istore_1: {
                    locals.set(1, stack.pop());
                    break;
                }
                case Instruction::istore_2: {
                    locals.set(2, stack.pop());
                    break;
                }
                case Instruction::istore_3: {
                    locals.set(3, stack.pop());
                    break;
                }
                case Instruction::isub: {
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(value1 - value2));
                    break;
                }
                case Instruction::iushr: {
                    unimplemented();
                    break;
                }
                case Instruction::ixor: {
                    unimplemented();
                    break;
                }
                    // LONG
                case Instruction::l2d:
                case Instruction::l2f:
                case Instruction::l2i:
                case Instruction::ladd:
                case Instruction::laload:
                case Instruction::land:
                case Instruction::lastore:
                case Instruction::lcmp:
                case Instruction::lconst_0:
                case Instruction::lconst_1:
                case Instruction::ldiv: {
                    unimplemented();
                    break;
                }
                case Instruction::lload: {
                    let instruction = _instruction.get<Instruction::lload>();
                    stack.push(locals.get(Int(instruction.index)).copy());
                    break;
                }
                case Instruction::lload_0: {
                    stack.push(locals.get(0).copy());
                    break;
                }
                case Instruction::lload_1: {
                    stack.push(locals.get(1).copy());
                    break;
                }
                case Instruction::lload_2: {
                    stack.push(locals.get(2).copy());
                    break;
                }
                case Instruction::lload_3: {
                    stack.push(locals.get(3).copy());
                    break;
                }
                case Instruction::lmul:
                case Instruction::lneg:
                case Instruction::lor:
                case Instruction::lrem:
                case Instruction::lreturn:
                case Instruction::lshl:
                case Instruction::lshr:
                case Instruction::lstore:
                case Instruction::lstore_0:
                case Instruction::lstore_1:
                case Instruction::lstore_2:
                case Instruction::lstore_3:
                case Instruction::lsub:
                case Instruction::lushr:
                case Instruction::lxor: {
                    unimplemented();
                    break;
                }
                    // FLOAT
                case Instruction::f2d:
                case Instruction::f2i:
                case Instruction::f2l:
                case Instruction::fadd:
                case Instruction::faload:
                case Instruction::fastore:
                case Instruction::fcmpg:
                case Instruction::fcmpl:
                case Instruction::fconst_0:
                case Instruction::fconst_1:
                case Instruction::fconst_2:
                case Instruction::fdiv:
                case Instruction::fload:
                case Instruction::fload_0:
                case Instruction::fload_1:
                case Instruction::fload_2:
                case Instruction::fload_3:
                case Instruction::fmul:
                case Instruction::fneg:
                case Instruction::frem:
                case Instruction::freturn:
                case Instruction::fstore:
                case Instruction::fstore_0:
                case Instruction::fstore_1:
                case Instruction::fstore_2:
                case Instruction::fstore_3:
                case Instruction::fsub: {
                    unimplemented();
                    break;
                }
                    // DOUBLE
                case Instruction::d2f:
                case Instruction::d2i:
                case Instruction::d2l:
                case Instruction::dadd:
                case Instruction::daload:
                case Instruction::dastore:
                case Instruction::dcmpg:
                case Instruction::dcmpl:
                case Instruction::dconst_0:
                case Instruction::dconst_1:
                case Instruction::ddiv:
                case Instruction::dload:
                case Instruction::dload_0:
                case Instruction::dload_1:
                case Instruction::dload_2:
                case Instruction::dload_3:
                case Instruction::dmul:
                case Instruction::dneg:
                case Instruction::drem:
                case Instruction::dreturn:
                case Instruction::dstore:
                case Instruction::dstore_0:
                case Instruction::dstore_1:
                case Instruction::dstore_2:
                case Instruction::dstore_3:
                case Instruction::dsub: {
                    unimplemented();
                    break;
                }
            }
        }
    }
}
