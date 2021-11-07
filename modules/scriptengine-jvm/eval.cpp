#include "scriptengine.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

#include "classfile.hpp"

namespace scriptengine::jvm {
    using namespace tier0;
    using namespace tier1;
}

namespace scriptengine::jvm {
    Evaluator::~Evaluator() {}

    void eval(MethodHandle mh, mut_ref<Evaluator> evaluator, Frame frame) {
        var &locals = frame.locals;
        var &stack = frame.stack;
        {
            let nLocals = 100; // fixme
            locals.ensure(nLocals);
            locals._size(nLocals);
        }
        let pool = mh.handle_.handle_->constantPool;
        let code = load_code(mh);
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
                    let refClass = pool.get(instruction.index - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    stack.push(evaluator._newarray(refClassName.string.value_, count));
                    break;
                }
                case Instruction::arraylength: {
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(evaluator.arraysize(self));
                    break;
                }
                case Instruction::checkcast: {
                    unimplemented();
                    break;
                }
                case Instruction::getfield: {
                    let instruction = _instruction.get<Instruction::getfield>();
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Fieldref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    (void) refClassName;
                    (void) refType;
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    stack.push(evaluator.getinstance(self, refName.string.value_));
                    break;
                }
                case Instruction::getstatic: {
                    let instruction = _instruction.get<Instruction::getstatic>();
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Fieldref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    (void) refType;
                    stack.push(evaluator.getstatic(refClassName.string.value_, refName.string.value_));
                    break;
                }
                case Instruction::instanceof: {
                    unimplemented();
                    break;
                }
                case Instruction::multianewarray: {
                    unimplemented();
                    break;
                }
                case Instruction::_new: {
                    let instruction = _instruction.get<Instruction::_new>();
                    let refClass = pool.get(instruction.index - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    stack.push(evaluator._new(refClassName.string.value_));
                    break;
                }
                case Instruction::newarray: {
                    unimplemented();
                    break;
                }
                case Instruction::putfield: {
                    let instruction = _instruction.get<Instruction::putfield>();
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Fieldref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    (void) refClassName;
                    (void) refType;
                    var value = stack.pop();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    evaluator.putinstance(self, refName.string.value_, move(value));
                    break;
                }
                case Instruction::putstatic: {
                    let instruction = _instruction.get<Instruction::putstatic>();
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Fieldref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    (void) refType;
                    evaluator.putstatic(refClassName.string.value_, refName.string.value_, stack.pop());
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
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Methodref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    evaluator.invokespecial(
                            refClassName.string.value_, refName.string.value_, refType.string.value_,
                            frame
                    );
                    break;
                }
                case Instruction::invokestatic: {
                    let instruction = _instruction.get<Instruction::invokestatic>();
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Methodref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    evaluator.invokestatic(
                            refClassName.string.value_, refName.string.value_, refType.string.value_,
                            frame
                    );
                    break;
                }
                case Instruction::invokevirtual: {
                    let instruction = _instruction.get<Instruction::invokevirtual>();
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Methodref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    evaluator.invokevirtual(
                            refClassName.string.value_, refName.string.value_, refType.string.value_,
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
                case Instruction::ldc: {
                    let instruction = _instruction.get<Instruction::ldc>();
                    let constant = pool.get(instruction.index - 1).variant_.get<Constant::String>();
                    let constantValue = pool.get(constant.stringIndex - 1).variant_.get<Constant::Utf8>();
                    stack.push(Stack::Value::of<Stack::ValueKind::String>(constantValue.string.value_));
                    break;
                }
                case Instruction::ldc_w: {
                    let instruction = _instruction.get<Instruction::ldc_w>();
                    let constant = pool.get(instruction.index - 1).variant_.get<Constant::String>();
                    let constantValue = pool.get(constant.stringIndex - 1).variant_.get<Constant::Utf8>();
                    stack.push(Stack::Value::of<Stack::ValueKind::String>(constantValue.string.value_));
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
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue != Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                case Instruction::ifnull: {
                    let instruction = _instruction.get<Instruction::ifnull>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue == Int(0)) {
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
                    stack.push(evaluator.arrayget(self, index));
                    break;
                }
                case Instruction::aastore: {
                    var value = stack.pop();
                    var index = stack.pop().get<Stack::ValueKind::Int>();
                    var self = stack.pop().get<Stack::ValueKind::Reference>();
                    evaluator.arrayset(self, index, move(value));
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
                    unimplemented();
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
                    unimplemented();
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
                    unimplemented();
                    break;
                }
                case Instruction::i2s: {
                    unimplemented();
                    break;
                }
                case Instruction::iadd: {
                    var value2 = stack.pop().get<Stack::ValueKind::Int>();
                    var value1 = stack.pop().get<Stack::ValueKind::Int>();
                    stack.push(Stack::Value::of<Stack::ValueKind::Int>(value1 + value2));
                    break;
                }
                case Instruction::iaload: {
                    unimplemented();
                    break;
                }
                case Instruction::iand: {
                    unimplemented();
                    break;
                }
                case Instruction::iastore: {
                    unimplemented();
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
                    unimplemented();
                    break;
                }
                case Instruction::irem: {
                    unimplemented();
                    break;
                }
                case Instruction::ireturn: {
                    unimplemented();
                    break;
                }
                case Instruction::ishl: {
                    unimplemented();
                    break;
                }
                case Instruction::ishr: {
                    unimplemented();
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
                    unimplemented();
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
                case Instruction::ldiv:
                case Instruction::lload:
                case Instruction::lload_0:
                case Instruction::lload_2:
                case Instruction::lload_3:
                case Instruction::lload_4:
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
