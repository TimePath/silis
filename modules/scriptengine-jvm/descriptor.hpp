#pragma once

#include "../tier2/tier2.hpp"

namespace scriptengine::jvm {
    using namespace tier2;
}

namespace scriptengine::jvm {

    // https://docs.oracle.com/javase/specs/jvms/se17/html/jvms-4.html#jvms-4.3
    struct DescriptorParser {
        enum class AtomKind {
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

        struct Atom {
            AtomKind kind_;
            PAD(4)
            StringSpan value_;
            Int begin_;
            Int end_;

            explicit Atom(AtomKind kind, StringSpan value, Int begin, Int end)
                    : kind_(kind), value_(value), begin_(begin), end_(end) {}
        };

        StringSpan input;
        Int idx = 0;
        PAD(4)
        List<Atom> output = List<Atom>();

        static List<Atom> parseMethod(StringSpan input) {
            var state = DescriptorParser{
                    .input = input,
            };
            state.methodDescriptor();
            return move(state.output);
        }

        [[maybe_unused]]
        static List<Atom> parseField(StringSpan input) {
            var state = DescriptorParser{
                    .input = input,
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

        struct Name {
            Int begin_;
            Int end_;
        };

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
                output.add(Atom(AtomKind::Byte, input, begin, idx));
                return true;
            }
            if (require('C')) {
                output.add(Atom(AtomKind::Char, input, begin, idx));
                return true;
            }
            if (require('D')) {
                output.add(Atom(AtomKind::Double, input, begin, idx));
                return true;
            }
            if (require('F')) {
                output.add(Atom(AtomKind::Float, input, begin, idx));
                return true;
            }
            if (require('I')) {
                output.add(Atom(AtomKind::Int, input, begin, idx));
                return true;
            }
            if (require('J')) {
                output.add(Atom(AtomKind::Long, input, begin, idx));
                return true;
            }
            if (require('S')) {
                output.add(Atom(AtomKind::Short, input, begin, idx));
                return true;
            }
            if (require('Z')) {
                output.add(Atom(AtomKind::Boolean, input, begin, idx));
                return true;
            }
            return false;
        }

        Boolean objectType() {
            if (!require('L')) return false;
            var begin = idx;
            if (!className()) return error();
            output.add(Atom(AtomKind::Reference, input, begin, idx));
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
            output.add(Atom(AtomKind::Void, input, begin, idx));
            return true;
        }
    };

}
