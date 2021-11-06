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
        var stack = Stack();
        var pc = Int(0);
        var jump = [&](Int relative) { pc = pc + relative - 1; };
        for (; run; pc = pc + 1) {
            let _instruction = code.code_.get(pc).variant_;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
            switch (_instruction.index()) {
#pragma clang diagnostic pop
                default: {
                    run = false;
                    break;
                }
                case Instruction::_return: {
                    run = false;
                    break;
                }
                    // CLASS
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
                case Instruction::ldc: {
                    let instruction = _instruction.get<Instruction::ldc>();
                    let constant = pool.get(instruction.index - 1).variant_.get<Constant::String>();
                    let constantValue = pool.get(constant.stringIndex - 1).variant_.get<Constant::Utf8>();
                    stack.push(Stack::Value::of<Stack::ValueKind::String>(constantValue.string.value_));
                    break;
                }
                    // BRANCH
                case Instruction::_goto: {
                    let instruction = _instruction.get<Instruction::ifeq>();
                    jump(Int(instruction.branch));
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
                case Instruction::ifne: {
                    let instruction = _instruction.get<Instruction::ifne>();
                    var value = stack.pop();
                    var intValue = value.get<Stack::ValueKind::Int>();
                    if (intValue != Int(0)) {
                        jump(Int(instruction.branch));
                    }
                    break;
                }
                    // REFERENCE
                    // BYTE
                    // CHAR
                    // SHORT
                    // INT
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
                    // LONG
                    // FLOAT
                    // DOUBLE
            }
        }
    }
}
