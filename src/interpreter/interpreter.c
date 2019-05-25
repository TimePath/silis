#include <system.h>
#include "interpreter.h"

#include <lib/fs.h>
#include <lib/stdio.h>

#include <parser/lexer.h>
#include <parser/parser.h>

#include "eval.h"

void InterpreterFile_delete(InterpreterFile *self)
{
    Allocator *allocator = self->allocator;
    FilePath_delete(&self->path);
    free(self->content);
    Vector_delete(Token, &self->tokens);
    Vector_delete(Node, &self->nodes);
    free(self);
}

const InterpreterFile *Interpreter_lookup_file(const Interpreter *self, InterpreterFileRef ref)
{
    assert(ref.id && "file exists");
    const InterpreterFile *file = *Vector_at(&self->compilation.files, ref.id - 1);
    return file;
}

const Token *Interpreter_lookup_file_token(const Interpreter *self, InterpreterFileTokenRef ref)
{
    assert(ref.token.id && "token exists");
    const InterpreterFile *file = Interpreter_lookup_file(self, ref.file);
    const Token *token = Vector_at(&file->tokens, ref.token.id - 1);
    return token;
}

const Node *Interpreter_lookup_file_node(const Interpreter *self, InterpreterFileNodeRef ref)
{
    assert(ref.node.id && "node exists");
    const InterpreterFile *file = Interpreter_lookup_file(self, ref.file);
    const Node *node = Vector_at(&file->nodes, ref.node.id - 1);
    return node;
}

InterpreterFileNodeRef Interpreter_lookup_node_ref(const Interpreter *self, InterpreterFileNodeRef ref)
{
    const Node *it = Interpreter_lookup_file_node(self, ref);
    if (it->kind.val == Node_Ref) {
        ref.node.id = it->u.Ref.value;
        assert(Interpreter_lookup_file_node(self, ref)->kind.val == Node_ListBegin && "references refer to lists");
    }
    return ref;
}

InterpreterFileRef Interpreter_load(Interpreter *self, FileSystem *fs, FilePath path)
{
    Allocator *allocator = self->allocator;
    String fileStr;
    uint8_t *read = fs_read_all(allocator, fs, path, &fileStr);
    if (!read) {
        assert(read && "read from file");
        return (InterpreterFileRef) {0};
    }
    return Interpreter_read(self, fileStr, path);
}

InterpreterFileRef Interpreter_read(Interpreter *self, String file, FilePath path)
{
    Allocator *allocator = self->allocator;
    InterpreterFileRef ret = {.id = Vector_size(&self->compilation.files) + 1};

    if (self->compilation.flags.print_lex) {
        fprintf_s(self->compilation.debug, STR("LEX:\n---\n"));
    }
    Result(Vector(Token), ParserError) lex = silis_parser_lex((silis_parser_lex_input) {
        .allocator = allocator,
        .source = file,
    });
    if (!lex.is.ok) {
        ParserError_print(lex.ret.err, self->compilation.debug);
        unreachable();
        return (InterpreterFileRef) {0};
    }
    Vector(Token) tokens = lex.ret.val;
    if (self->compilation.flags.print_lex) {
        silis_parser_print_tokens(Vector_toSlice(Token, &tokens), self->compilation.debug, allocator);
        fprintf_s(self->compilation.debug, STR("\n\n"));
    }

    if (self->compilation.flags.print_parse) {
        fprintf_s(self->compilation.debug, STR("PARSE:\n-----\n"));
    }
    silis_parser_parse_output parse = silis_parser_parse((silis_parser_parse_input) {
            .allocator = allocator,
            .tokens = Vector_toSlice(Token, &tokens),
    });
    if (self->compilation.flags.print_parse) {
        silis_parser_print_nodes(Vector_toSlice(Node, &parse.nodes), self->compilation.debug, allocator);
        fprintf_s(self->compilation.debug, STR("\n"));
    }
    InterpreterFile *f = malloc(sizeof(*f));
    *f = (InterpreterFile) {
            .allocator = allocator,
            .path = path,
            .content = Slice_data_mut(&file.bytes),
            .tokens = tokens,
            .nodes = parse.nodes,
            .entry = {.file = ret, .node = {.id = parse.root_id}},
    };
    Vector_push(&self->compilation.files, f);

    return ret;
}

void Interpreter_eval(Interpreter *self, InterpreterFileRef file)
{
    Allocator *allocator = self->allocator;
    const InterpreterFile *f = Interpreter_lookup_file(self, file);
    FilePath dir = fs_dirname(allocator, f->path);
    fs_dirtoken state = fs_pushd(allocator, dir);
    FilePath_delete(&dir);

    if (self->compilation.flags.print_eval) {
        fprintf_s(self->compilation.debug, STR("EVAL:\n----\n"));
    }
    do_eval((eval_input) {
            .interpreter = self,
            .entry = f->entry,
    });
    if (self->compilation.flags.print_eval) {
        fprintf_s(self->compilation.debug, STR("\n"));
    }

    fs_popd(state);
}
