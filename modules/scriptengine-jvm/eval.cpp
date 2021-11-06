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
        var stack = Stack();
        var run = Boolean(true);
        for (var pc = Int(-1); (void) (pc = pc + 1), run;) {
            let v = code.code_.get(pc).variant_;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
            switch (v.index()) {
#pragma clang diagnostic pop
                case Instruction::getstatic: {
                    let instruction = v.get<Instruction::getstatic>();
                    let ref = pool.get(instruction.index - 1).variant_.get<Constant::Fieldref>();
                    let refClass = pool.get(ref.classIndex - 1).variant_.get<Constant::Class>();
                    let refClassName = pool.get(refClass.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refNameAndType = pool.get(ref.nameAndTypeIndex - 1).variant_.get<Constant::NameAndType>();
                    let refName = pool.get(refNameAndType.nameIndex - 1).variant_.get<Constant::Utf8>();
                    let refType = pool.get(refNameAndType.descriptorIndex - 1).variant_.get<Constant::Utf8>();
                    (void) refType;
                    stack.push(Stack::Value::of<Stack::ValueKind::Reference>(
                            evaluator.getstatic(refClassName.string.value_, refName.string.value_)
                    ));
                    break;
                }
                case Instruction::ldc: {
                    let instruction = v.get<Instruction::ldc>();
                    let constant = pool.get(instruction.index - 1).variant_.get<Constant::String>();
                    let constantValue = pool.get(constant.stringIndex - 1).variant_.get<Constant::Utf8>();
                    stack.push(Stack::Value::of<Stack::ValueKind::String>(constantValue.string.value_));
                    break;
                }
                case Instruction::invokestatic: {
                    let instruction = v.get<Instruction::invokestatic>();
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
                    let instruction = v.get<Instruction::invokevirtual>();
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
                case Instruction::_return: {
                    run = false;
                    break;
                }
                default: {
                    run = false;
                    break;
                }
            }
        }
    }
}
