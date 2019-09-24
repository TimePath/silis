#include <prelude.h>
#include "_.h"

void Target_file_begin(Target *target, Interpreter *interpreter, Ref(InterpreterFile) file_ref, Vector(compile_file) *files)
{
    target->_file_begin(target, interpreter, file_ref, files);
}

void Target_file_end(Target *target, Interpreter *interpreter, const compile_file *file)
{
    target->_file_end(target, interpreter, file);
}

void Target_func_forward(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name)
{
    target->_func_forward(target, interpreter, file, T, name);
}

void Target_func_declare(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T, String name, const String argnames[])
{
    target->_func_declare(target, interpreter, file, T, name, argnames);
}

void Target_var_begin(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T)
{
    target->_var_begin(target, interpreter, file, T);
}

void Target_var_end(Target *target, Interpreter *interpreter, const compile_file *file, Ref(Type) T)
{
    target->_var_end(target, interpreter, file, T);
}

void Target_identifier(Target *target, Interpreter *interpreter, const compile_file *file, String name)
{
    target->_identifier(target, interpreter, file, name);
}
