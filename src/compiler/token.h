#pragma once

#include "token-fwd.h"

#include <lib/fs.h>
#include <lib/string.h>
#include <lib/vector.h>

typedef enum {
    TOKEN_INVALID,
    /**
     * (
     */ TOKEN_LIST_BEGIN,
    /**
     * )
     */ TOKEN_LIST_END,
    /**
     * A word
     */ TOKEN_ATOM,
    /**
     * An integer
     */ TOKEN_INTEGRAL,
    /**
     * A string
     */ TOKEN_STRING,
} token_e;

struct token_s {
    token_e kind;
    PADDING(4)
    union {
        /// TOKEN_ATOM
        struct {
            String value;
        } atom;
        /// TOKEN_INTEGRAL
        struct {
            size_t value;
        } integral;
        /// TOKEN_STRING
        struct {
            String value;
        } string;
    } u;
};

#define token_t_delete(self) ((void) (self))

void token_print(Allocator *allocator, File *f, Slice(token_t) it);
