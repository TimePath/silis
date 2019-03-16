#pragma once

#include <lib/buffer.h>

#include "node.h"

typedef struct {
    struct compilation_s *compilation;
    struct types_s *types;
    struct symbols_s *symbols;
    // todo: bind to intrinsic instances
    File *out;
    // todo: bind to intrinsic instances
    Buffer *preludeBuf;
    File *prelude;
} Env;
