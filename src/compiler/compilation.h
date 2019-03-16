#pragma once

#include "compilation-fwd.h"

#include <lib/string.h>
#include <lib/vector.h>

#include "token-fwd.h"
#include "node-fwd.h"
#include "env.h"

struct compilation_s {
    File *debug;
    Vector(compilation_file_ptr_t) files;
    const struct {
        bool print_parse : 1;
        bool print_flatten : 1;
        bool print_eval: 1;
        uint8_t padding : 5;
    } flags;
    uint8_t padding[7];
};

struct compilation_file_s {
    String path;
    uint8_t *content;
    Vector(token_t) tokens;
    Vector(node_t) nodes;
    compilation_node_ref entry;
};

compilation_file_ref compilation_include(compilation_t *self, String path);

void compilation_begin(compilation_t *self, compilation_file_ref file, Env env);
