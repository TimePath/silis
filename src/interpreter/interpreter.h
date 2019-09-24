#pragma once

#include <lib/string.h>
#include <lib/vector.h>

#include <parser/token.h>
#include <parser/node.h>

#include "types.h"

Ref_instantiate(InterpreterFile, size_t);

typedef struct {
    Ref(InterpreterFile) file;
    Ref(Token) token;
} InterpreterFileTokenRef;

typedef struct {
    Ref(InterpreterFile) file;
    Ref(Node) node;
} InterpreterFileNodeRef;

#define InterpreterFileNodeRef_delete(self) ((void) (self))

Slice_instantiate(InterpreterFileNodeRef);
Vector_instantiate(InterpreterFileNodeRef);

typedef struct {
    Allocator *allocator;
    FilePath path;
    uint8_t *content;
    Vector(Token) tokens;
    Vector(Node) nodes;
    InterpreterFileNodeRef entry;
} InterpreterFile;

void InterpreterFile_delete(InterpreterFile *self);

typedef InterpreterFile *InterpreterFilePtr;

#define InterpreterFilePtr_delete(self) InterpreterFile_delete(*self)

Slice_instantiate(InterpreterFilePtr);
Vector_instantiate(InterpreterFilePtr);

typedef struct {
    Allocator *allocator;
    FileSystem *fs_in;
    struct {
        File *debug;
        Vector(InterpreterFilePtr) files;
        struct {
            bool print_lex : 1;
            bool print_parse : 1;
            bool print_eval: 1;
            BIT_PADDING(uint8_t, 5)
        } flags;
        PADDING(7)
    } compilation;
    Types *types;
    struct Symbols *symbols;
    // todo: bind to intrinsic instances
    File *out;
} Interpreter;

const InterpreterFile *Interpreter_lookup_file(const Interpreter *self, Ref(InterpreterFile) ref);

const Token *Interpreter_lookup_file_token(const Interpreter *self, InterpreterFileTokenRef ref);

const Node *Interpreter_lookup_file_node(const Interpreter *self, InterpreterFileNodeRef ref);

InterpreterFileNodeRef Interpreter_lookup_node_ref(const Interpreter *self, InterpreterFileNodeRef ref);

Ref(InterpreterFile) Interpreter_load(Interpreter *self, FileSystem *fs, FilePath path);

Ref(InterpreterFile) Interpreter_read(Interpreter *self, String file, FilePath path);

void Interpreter_eval(Interpreter *self, Ref(InterpreterFile) file);
