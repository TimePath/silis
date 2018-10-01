#include <system.h>
#include "compilation.h"

#include <lib/fs.h>
#include <lib/stdio.h>

#include <compiler/phases/01-parse/parse.h>
#include <compiler/phases/02-flatten/flatten.h>
#include <compiler/phases/03-eval/eval.h>

const compilation_file_t *compilation_file(const compilation_t *self, compilation_file_ref ref)
{
    assert(ref.id && "file exists");
    const compilation_file_t *file = Vector_data(&self->files)[ref.id - 1];
    return file;
}

const token_t *compilation_token(const compilation_t *self, compilation_token_ref ref)
{
    assert(ref.token.id && "token exists");
    const compilation_file_t *file = compilation_file(self, ref.file);
    const token_t *token = &Vector_data(&file->tokens)[ref.token.id - 1];
    return token;
}

const node_t *compilation_node(const compilation_t *self, compilation_node_ref ref)
{
    assert(ref.node.id && "node exists");
    const compilation_file_t *file = compilation_file(self, ref.file);
    const node_t *node = &Vector_data(&file->nodes)[ref.node.id - 1];
    return node;
}

// todo: remove
compilation_node_ref compilation_node_find(const compilation_t *self, const node_t *node)
{
    Slice_loop(&Vector_toSlice(compilation_file_ptr_t, &self->files), i) {
        compilation_file_t *f = Vector_data(&self->files)[i];
        const Vector(node_t) *nodes = &f->nodes;
        if (node > Vector_data(nodes) && node < Vector_data(nodes) + Vector_size(nodes)) {
            return (compilation_node_ref) {.file = {i + 1}, .node = {(size_t) (node - Vector_data(nodes)) + 1}};
        }
    }
    assert(false);
    return (compilation_node_ref) {.file = {0}, .node = {0}};
}

compilation_file_ref compilation_include(compilation_t *self, String path)
{
    String fileStr;
    bool read = fs_read(path, &fileStr);
    if (!read) {
        assert(read && "read from file");
        return (compilation_file_ref) {0};
    }
    compilation_file_ref ret = {.id = Vector_size(&self->files) + 1};

    if (self->flags.print_parse) {
        fprintf_s(self->debug, STR("PARSE:\n-----\n"));
    }
    parse_output parse = do_parse((parse_input) {
            .source = fileStr,
    });
    if (self->flags.print_parse) {
        token_print(self->debug, Vector_toSlice(token_t, &parse.tokens));
        fprintf_s(self->debug, STR("\n\n"));
    }

    if (self->flags.print_flatten) {
        fprintf_s(self->debug, STR("FLATTEN:\n-------\n"));
    }
    flatten_output flatten = do_flatten((flatten_input) {
            .file = ret,
            .tokens = parse.tokens,
    });
    if (self->flags.print_flatten) {
        node_print(self->debug, Vector_toSlice(node_t, &flatten.nodes));
        fprintf_s(self->debug, STR("\n"));
    }
    compilation_file_t *file = realloc(NULL, sizeof(*file));
    *file = (compilation_file_t) {
            .path = path,
            .tokens = parse.tokens,
            .nodes = flatten.nodes,
            .entry = {.file = ret, .node = {.id = flatten.entry + 1}},
    };
    Vector_push(&self->files, file);

    return ret;
}

void compilation_begin(compilation_t *self, compilation_file_ref file, Env env)
{
    const compilation_file_t *f = compilation_file(self, file);
    fs_dirtoken state = fs_pushd_dirname(f->path);

    if (self->flags.print_eval) {
        fprintf_s(self->debug, STR("EVAL:\n----\n"));
    }
    do_eval((eval_input) {
            .env = env,
            .entry = compilation_node(self, compilation_file(self, file)->entry),
    });
    if (self->flags.print_eval) {
        fprintf_s(self->debug, STR("\n"));
    }

    fs_popd(state);
}
