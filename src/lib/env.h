#pragma once

#include "allocator.h"
#include "fs.h"
#include "string.h"
#include "slice.h"

typedef struct {
    Slice(String) args;
    File *stdout;
    FileSystem *fs;
    Allocator *allocator;
} Env;
