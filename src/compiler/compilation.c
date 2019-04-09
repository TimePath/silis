#include <system.h>
#include "compilation.h"

#include <lib/fs.h>
#include <lib/stdio.h>

#include <compiler/phases/01-lex/lex.h>
#include <compiler/phases/02-parse/parse.h>
#include <compiler/phases/03-eval/eval.h>

void compilation_file_t_delete(compilation_file_t *self)
{
    FilePath_delete(&self->path);
    free(self->content);
    Vector_delete(token_t, &self->tokens);
    Vector_delete(node_t, &self->nodes);
    free(self);
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

compilation_file_ref compilation_include(compilation_t *self, FilePath path)
{
    String fileStr;
    uint8_t *read = fs_read_all(path, &fileStr);
    if (!read) {
        assert(read && "read from file");
        return (compilation_file_ref) {0};
    }
    compilation_file_ref ret = {.id = Vector_size(&self->files) + 1};

    if (self->flags.print_parse) {
        fprintf_s(self->debug, STR("PARSE:\n-----\n"));
    }
    lex_output lex = do_lex((lex_input) {
            .source = fileStr,
    });
    if (self->flags.print_parse) {
        token_print(self->debug, Vector_toSlice(token_t, &lex.tokens));
        fprintf_s(self->debug, STR("\n\n"));
    }

    if (self->flags.print_flatten) {
        fprintf_s(self->debug, STR("FLATTEN:\n-------\n"));
    }
    parse_output parse = do_parse((parse_input) {
            .file = ret,
            .tokens = lex.tokens,
    });
    if (self->flags.print_flatten) {
        node_print(self->debug, Vector_toSlice(node_t, &parse.nodes));
        fprintf_s(self->debug, STR("\n"));
    }
    compilation_file_t *file = realloc(NULL, sizeof(*file));
    *file = (compilation_file_t) {
            .path = path,
            .content = read,
            .tokens = lex.tokens,
            .nodes = parse.nodes,
            .entry = {.file = ret, .node = {.id = parse.root}},
    };
    Vector_push(&self->files, file);

    return ret;
}

void compilation_begin(compilation_t *self, compilation_file_ref file, Env env)
{
    const compilation_file_t *f = compilation_file(self, file);
    FilePath dir = fs_dirname(f->path);
    fs_dirtoken state = fs_pushd(dir);
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
