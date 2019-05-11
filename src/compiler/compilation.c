#include <system.h>
#include "compilation.h"

#include <lib/fs.h>
#include <lib/stdio.h>

#include <parser/lex.h>
#include <parser/parse.h>
#include <compiler/phases/03-eval/eval.h>

void compilation_file_t_delete(compilation_file_t *self)
{
    Allocator *allocator = self->allocator;
    FilePath_delete(&self->path);
    free(self->content);
    Vector_delete(token_t, &self->tokens);
    Vector_delete(node_t, &self->nodes);
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

const compilation_file_t *compilation_file(const compilation_t *self, compilation_file_ref ref)
{
    assert(ref.id && "file exists");
    const compilation_file_t *file = *Vector_at(&self->files, ref.id - 1);
    return file;
}

const token_t *compilation_token(const compilation_t *self, compilation_token_ref ref)
{
    assert(ref.token.id && "token exists");
    const compilation_file_t *file = compilation_file(self, ref.file);
    const token_t *token = Vector_at(&file->tokens, ref.token.id - 1);
    return token;
}

const node_t *compilation_node(const compilation_t *self, compilation_node_ref ref)
{
    assert(ref.node.id && "node exists");
    const compilation_file_t *file = compilation_file(self, ref.file);
    const node_t *node = Vector_at(&file->nodes, ref.node.id - 1);
    return node;
}

compilation_file_ref compilation_include(Allocator *allocator, compilation_t *self, FileSystem *fs, FilePath path)
{
    String fileStr;
    uint8_t *read = fs_read_all(allocator, fs, path, &fileStr);
    if (!read) {
        assert(read && "read from file");
        return (compilation_file_ref) {0};
    }
    compilation_file_ref ret = {.id = Vector_size(&self->files) + 1};

    if (self->flags.print_lex) {
        fprintf_s(self->debug, STR("LEX:\n---\n"));
    }
    lex_output lex = do_lex((lex_input) {
        .allocator = allocator,
        .source = fileStr,
    });
    if (self->flags.print_lex) {
        token_print(allocator, self->debug, Vector_toSlice(token_t, &lex.tokens));
        fprintf_s(self->debug, STR("\n\n"));
    }

    if (self->flags.print_parse) {
        fprintf_s(self->debug, STR("PARSE:\n-----\n"));
    }
    parse_output parse = do_parse((parse_input) {
            .allocator = allocator,
            .tokens = lex.tokens,
    });
    if (self->flags.print_parse) {
        node_print(allocator, self->debug, Vector_toSlice(node_t, &parse.nodes));
        fprintf_s(self->debug, STR("\n"));
    }
    compilation_file_t *file = malloc(sizeof(*file));
    *file = (compilation_file_t) {
            .allocator = allocator,
            .path = path,
            .content = read,
            .tokens = lex.tokens,
            .nodes = parse.nodes,
            .entry = {.file = ret, .node = {.id = parse.root_id}},
    };
    Vector_push(&self->files, file);

    return ret;
}

void compilation_begin(Allocator *allocator, compilation_t *self, compilation_file_ref file, Env env)
{
    const compilation_file_t *f = compilation_file(self, file);
    FilePath dir = fs_dirname(allocator, f->path);
    fs_dirtoken state = fs_pushd(allocator, dir);
    FilePath_delete(&dir);

    if (self->flags.print_eval) {
        fprintf_s(self->debug, STR("EVAL:\n----\n"));
    }
    do_eval((eval_input) {
            .env = env,
            .entry = f->entry,
    });
    if (self->flags.print_eval) {
        fprintf_s(self->debug, STR("\n"));
    }

    fs_popd(state);
}
