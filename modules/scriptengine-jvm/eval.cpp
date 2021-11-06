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

    void eval(MethodHandle mh, mut_ref<Evaluator> evaluator) {
        let pool = mh.handle_.handle_->constantPool;
        let code = load_code(mh);
        var run = Boolean(true);
        var unimplemented = [&]() {
            run = false;
        };
        var stack = Stack();
        var pc = Int(0);
        var jump = [&](Int relative) { pc = pc + relative - 1; };
        for (; run; pc = pc + 1) {
            let _instruction = code.code_.get(pc).variant_;
            switch (_instruction.index()) {
                case Instruction::Invalid: {
                    unimplemented();
                    break;
                }
                case Instruction::athrow:
                case Instruction::monitorenter:
                case Instruction::monitorexit:
                case Instruction::nop: {
                    unimplemented();
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
                case Instruction::anewarray:
                case Instruction::arraylength:
                case Instruction::checkcast:
                case Instruction::getfield: {
                    unimplemented();
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
                case Instruction::instanceof:
                case Instruction::multianewarray:
                case Instruction::_new:
                case Instruction::newarray:
                case Instruction::putfield: {
                    unimplemented();
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
                case Instruction::invokedynamic:
                case Instruction::invokeinterface:
                case Instruction::invokespecial: {
                    unimplemented();
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
                            stack
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
                            stack
                    );
                    break;
                }
                    // STACK
                case Instruction::dup:
                case Instruction::dup2:
                case Instruction::dup2_x1:
                case Instruction::dup2_x2:
                case Instruction::dup_x1:
                case Instruction::dup_x2: {
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
                case Instruction::ldc2_w:
                case Instruction::ldc_w:
                case Instruction::pop:
                case Instruction::swap: {
                    unimplemented();
                    break;
                }
                    // BRANCH
                case Instruction::_goto: {
                    let instruction = _instruction.get<Instruction::ifeq>();
                    jump(Int(instruction.branch));
                    break;
                }
                case Instruction::goto_w:
                case Instruction::if_acmpeq:
                case Instruction::if_acmpne:
                case Instruction::if_icmpeq:
                case Instruction::if_icmpge:
                case Instruction::if_icmpgt:
                case Instruction::if_icmple:
                case Instruction::if_icmplt:
                case Instruction::if_icmpne: {
                    unimplemented();
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
                case Instruction::ifge:
                case Instruction::ifgt:
                case Instruction::ifle:
                case Instruction::iflt: {
                    unimplemented();
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
                case Instruction::ifnonnull:
                case Instruction::ifnull:
                case Instruction::jsr:
                case Instruction::jsr_w:
                case Instruction::lookupswitch:
                case Instruction::ret:
                case Instruction::tableswitch: {
                    unimplemented();
                    break;
                }
                    // REFERENCE
                case Instruction::aaload:
                case Instruction::aastore:
                case Instruction::aconst_null:
                case Instruction::aload:
                case Instruction::aload_0:
                case Instruction::aload_1:
                case Instruction::aload_2:
                case Instruction::aload_3:
                case Instruction::areturn:
                case Instruction::astore:
                case Instruction::astore_0:
                case Instruction::astore_1:
                case Instruction::astore_2:
                case Instruction::astore_3: {
                    unimplemented();
                    break;
                }
                    // BYTE
                case Instruction::baload:
                case Instruction::bastore:
                case Instruction::bipush: {
                    unimplemented();
                    break;
                }
                    // CHAR
                case Instruction::caload:
                case Instruction::castore: {
                    unimplemented();
                    break;
                }
                    // SHORT
                case Instruction::saload:
                case Instruction::sastore:
                case Instruction::sipush: {
                    unimplemented();
                    break;
                }
                    // INT
                case Instruction::i2b:
                case Instruction::i2c:
                case Instruction::i2d:
                case Instruction::i2f:
                case Instruction::i2l:
                case Instruction::i2s:
                case Instruction::iadd:
                case Instruction::iaload:
                case Instruction::iand:
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
                case Instruction::idiv:
                case Instruction::iinc:
                case Instruction::iload:
                case Instruction::iload_0:
                case Instruction::iload_1:
                case Instruction::iload_2:
                case Instruction::iload_3:
                case Instruction::imul:
                case Instruction::ineg:
                case Instruction::ior:
                case Instruction::irem:
                case Instruction::ireturn:
                case Instruction::ishl:
                case Instruction::ishr:
                case Instruction::istore:
                case Instruction::istore_0:
                case Instruction::istore_1:
                case Instruction::istore_2:
                case Instruction::istore_3:
                case Instruction::isub:
                case Instruction::iushr:
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
