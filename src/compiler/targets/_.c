#include <system.h>
#include "_.h"

void Target_file_begin(Target *target, Env env, compilation_file_ref file_ref, Vector(compile_file) *files)
{
    target->_file_begin(target, env, file_ref, files);
}

void Target_file_end(Target *target, Env env, const compile_file *file)
{
    target->_file_end(target, env, file);
}

void Target_func_forward(Target *target, Env env, const compile_file *file, type_id T, String name)
{
    target->_func_forward(target, env, file, T, name);
}

void Target_func_declare(Target *target, Env env, const compile_file *file, type_id T, String name, const String argnames[])
{
    target->_func_declare(target, env, file, T, name, argnames);
}

void Target_var_begin(Target *target, Env env, const compile_file *file, type_id T)
{
    target->_var_begin(target, env, file, T);
}

void Target_var_end(Target *target, Env env, const compile_file *file, type_id T)
{
    target->_var_end(target, env, file, T);
}

void Target_identifier(Target *target, Env env, const compile_file *file, String name)
{
    target->_identifier(target, env, file, name);
}
