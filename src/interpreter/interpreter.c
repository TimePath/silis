#include <system.h>
#include "interpreter.h"

#include <lib/fs.h>
#include <lib/stdio.h>

#include <parser/lexer.h>
#include <parser/parser.h>

#include "eval.h"

void compilation_file_t_delete(compilation_file_t *self)
{
    Allocator *allocator = self->allocator;
    FilePath_delete(&self->path);
    free(self->content);
    Vector_delete(Token, &self->tokens);
    Vector_delete(Node, &self->nodes);
    free(self);
}

compile_file compile_file_new(Allocator *allocator, compilation_file_ref file, String ext, size_t flags)
{
    Buffer *content = malloc(sizeof(*content));
    *content = Buffer_new(allocator);
    return (compile_file) {
            .allocator = allocator,
            .file = file,
            .content = content,
            .out = Buffer_asFile(allocator, content),
            .ext = ext,
            .flags = flags,
    };
}

void compile_file_delete(compile_file *self)
{
    Allocator *allocator = self->allocator;
    fs_close(self->out);
    Buffer_delete(self->content);
    free(self->content);
}

const compilation_file_t *compilation_file(const Interpreter *self, compilation_file_ref ref)
{
    assert(ref.id && "file exists");
    const compilation_file_t *file = *Vector_at(&self->compilation.files, ref.id - 1);
    return file;
}

const Token *compilation_token(const Interpreter *self, compilation_token_ref ref)
{
    assert(ref.token.id && "token exists");
    const compilation_file_t *file = compilation_file(self, ref.file);
    const Token *token = Vector_at(&file->tokens, ref.token.id - 1);
    return token;
}

const Node *compilation_node(const Interpreter *self, compilation_node_ref ref)
{
    assert(ref.node.id && "node exists");
    const compilation_file_t *file = compilation_file(self, ref.file);
    const Node *node = Vector_at(&file->nodes, ref.node.id - 1);
    return node;
}

compilation_file_ref compilation_include(Allocator *allocator, Interpreter *self, FileSystem *fs, FilePath path)
{
    String fileStr;
    uint8_t *read = fs_read_all(allocator, fs, path, &fileStr);
    if (!read) {
        assert(read && "read from file");
        return (compilation_file_ref) {0};
    }
    compilation_file_ref ret = {.id = Vector_size(&self->compilation.files) + 1};

    if (self->compilation.flags.print_lex) {
        fprintf_s(self->compilation.debug, STR("LEX:\n---\n"));
    }
    Result(Vector(Token), ParserError) lex = silis_parser_lex((silis_parser_lex_input) {
        .allocator = allocator,
        .source = fileStr,
    });
    if (!lex.ok) {
        ParserError_print(lex.err, self->compilation.debug);
        unreachable();
        return (compilation_file_ref) {0};
    }
    Vector(Token) tokens = lex.val;
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
    compilation_file_t *file = malloc(sizeof(*file));
    *file = (compilation_file_t) {
            .allocator = allocator,
            .path = path,
            .content = read,
            .tokens = tokens,
            .nodes = parse.nodes,
            .entry = {.file = ret, .node = {.id = parse.root_id}},
    };
    Vector_push(&self->compilation.files, file);

    return ret;
}

void compilation_begin(Allocator *allocator, Interpreter *self, compilation_file_ref file, Interpreter *interpreter)
{
    const compilation_file_t *f = compilation_file(self, file);
    FilePath dir = fs_dirname(allocator, f->path);
    fs_dirtoken state = fs_pushd(allocator, dir);
    FilePath_delete(&dir);

    if (self->compilation.flags.print_eval) {
        fprintf_s(self->compilation.debug, STR("EVAL:\n----\n"));
    }
    do_eval((eval_input) {
            .interpreter = interpreter,
            .entry = f->entry,
    });
    if (self->compilation.flags.print_eval) {
        fprintf_s(self->compilation.debug, STR("\n"));
    }

    fs_popd(state);
}
