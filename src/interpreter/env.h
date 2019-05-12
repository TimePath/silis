#pragma once

#include <lib/allocator.h>
#include <lib/fs.h>

typedef struct {
    Allocator *allocator;
    FileSystem *fs_in;
    struct compilation_s *compilation;
    struct types_s *types;
    struct symbols_s *symbols;
    // todo: bind to intrinsic instances
    File *out;
} Env;
