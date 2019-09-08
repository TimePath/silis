#pragma once

#include <lib/allocator.h>

#include <interpreter/interpreter.h>

typedef struct {
    Allocator *allocator;
    InterpreterFileRef file;
    Buffer *content;
    File *out;
    String ext;
    /** output order, starting from 0 */
    size_t stage;
    /** role in code generation */
    size_t flags;
} compile_file;

typedef enum {
    FLAG_HEADER,
    FLAG_IMPL,
    FLAG_COUNT,
} file_flag;

Slice_instantiate(file_flag);

compile_file compile_file_new(InterpreterFileRef file, String ext, size_t stage, Slice(file_flag) flags, Allocator *allocator);

void compile_file_delete(compile_file *self);

Slice_instantiate(compile_file);
Vector_instantiate(compile_file);
