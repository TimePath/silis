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
