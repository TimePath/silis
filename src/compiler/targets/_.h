#pragma once

#include <interpreter/interpreter.h>
#include <interpreter/type.h>

#include <compiler/output.h>

typedef struct Target {
    void (*_file_begin)(struct Target *target, Interpreter *interpreter, Ref(InterpreterFile) file_ref, Vector(compile_file) *files);
    void (*_file_end)(struct Target *target, Interpreter *interpreter, const compile_file *file);
    void (*_func_forward)(struct Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name);
    void (*_func_declare)(struct Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name, const String argnames[]);
    void (*_var_begin)(struct Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T);
    void (*_var_end)(struct Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T);
    void (*_identifier)(struct Target *target, Interpreter *interpreter, const compile_file *file, String name);
} Target;

void Target_file_begin(Target *target, Interpreter *interpreter, Ref(InterpreterFile) file_ref, Vector(compile_file) *files);

void Target_file_end(Target *target, Interpreter *interpreter, const compile_file *file);

void Target_func_forward(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name);

void Target_func_declare(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name, const String argnames[]);

void Target_var_begin(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T);

void Target_var_end(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T);

void Target_identifier(Target *target, Interpreter *interpreter, const compile_file *file, String name);
