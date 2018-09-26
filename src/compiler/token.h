#pragma once

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

typedef struct {
    token_e kind;
    uint8_t padding[4];
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
} token_t;

Vector_instantiate(token_t);
Slice_instantiate(token_t);

void token_print(FILE *f, Slice(token_t) it);
