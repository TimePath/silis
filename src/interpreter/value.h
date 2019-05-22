#pragma once

#include <lib/vector.h>

#include "interpreter.h"
#include "nodelist.h"
#include "type.h"

typedef struct {
    TypeRef type;
    InterpreterFileNodeRef node;
    union {
        struct {
            void *value;
        } voidp;

        struct {
            size_t value;
        } integral;

        struct {
            String value;
        } string;

        struct {
            struct Intrinsic *value;
        } intrinsic;

        struct {
            InterpreterFileNodeRef value;
        } expr;

        struct {
            TypeRef value;
        } type;

        struct {
            InterpreterFileNodeRef value;
            InterpreterFileNodeRef arglist;
        } func;
    } u;
    struct {
        /// unknown value
        bool abstract : 1;
        /// intrinsic, can't be compiled as-is
        bool intrinsic : 1;
        /// native declaration (libc function, or other external symbol)
        bool native : 1;
        BIT_PADDING(uint8_t, 5)
    } flags;
    PADDING(7)
} Value;

Slice_instantiate(Value);
Vector_instantiate(Value);

Value Value_from(Interpreter *interpreter, InterpreterFileNodeRef n);
