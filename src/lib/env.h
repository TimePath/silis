#pragma once

#include "allocator.h"
#include "fs.h"
#include "string.h"
#include "slice.h"

typedef struct {
    Slice(String) args;
    File *out;
    FileSystem *fs;
    Allocator *allocator;
} Env;

size_t Env_run(size_t argc, native_string_t argv[], size_t (*run)(Env));
