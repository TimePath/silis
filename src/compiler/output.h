#pragma once

#include <lib/allocator.h>

#include <interpreter/interpreter.h>

typedef struct {
    Allocator *allocator;
    InterpreterFileRef file;
    Buffer *content;
    File *out;
    String ext;
    size_t flags;
} compile_file;

compile_file compile_file_new(Allocator *allocator, InterpreterFileRef file, String ext, size_t flags);

void compile_file_delete(compile_file *self);

Slice_instantiate(compile_file);
Vector_instantiate(compile_file);
