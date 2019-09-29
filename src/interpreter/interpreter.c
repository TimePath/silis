#include <prelude.h>
#include "interpreter.h"

#include <lib/fs.h>
#include <lib/misc.h>
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

const InterpreterFile *Interpreter_lookup_file(const Interpreter *self, Ref(InterpreterFilePtr) ref)
{
    assert(Ref_toBool(ref) && "file exists");
    const InterpreterFile *file = *Vector_at(&self->compilation.files, Ref_toIndex(ref));
    return file;
}

const Token *Interpreter_lookup_file_token(const Interpreter *self, InterpreterFileTokenRef ref)
{
    assert(Ref_toBool(ref.token) && "token exists");
    const InterpreterFile *file = Interpreter_lookup_file(self, ref.file);
    const Token *token = Vector_at(&file->tokens, Ref_toIndex(ref.token));
    return token;
}

const Node *Interpreter_lookup_file_node(const Interpreter *self, InterpreterFileNodeRef ref)
{
    assert(Ref_toBool(ref.node) && "node exists");
    const InterpreterFile *file = Interpreter_lookup_file(self, ref.file);
    const Node *node = Vector_at(&file->nodes, Ref_toIndex(ref.node));
    return node;
}

InterpreterFileNodeRef Interpreter_lookup_node_ref(const Interpreter *self, InterpreterFileNodeRef ref)
{
    const Node *it = Interpreter_lookup_file_node(self, ref);
    if (it->kind.val == Node_Ref) {
        ref.node = it->u.Ref.value;
        assert(Interpreter_lookup_file_node(self, ref)->kind.val == Node_ListBegin && "references refer to lists");
    }
    return ref;
}

Ref(InterpreterFilePtr) Interpreter_load(Interpreter *self, FileSystem *fs, FilePath path)
{
    Allocator *allocator = self->allocator;
    Slice(uint8_t) bytes;
    File *file = FileSystem_open(fs, path, STR("rb"));
    if (!file) {
        assert(file && "read from file");
        return (Ref(InterpreterFilePtr)) Ref_null;
    }
    bool read = File_read_all(file, &bytes, allocator);
    File_close(file);
    if (!read) {
        assert(read && "read from file");
        return (Ref(InterpreterFilePtr)) Ref_null;
    }
    String fileStr = String_fromSlice(bytes, ENCODING_DEFAULT);
    return Interpreter_read(self, fileStr, path);
}

Ref(InterpreterFilePtr) Interpreter_read(Interpreter *self, String file, FilePath path)
{
    Allocator *allocator = self->allocator;
    Ref(InterpreterFilePtr) ret = Vector_ref(&self->compilation.files, Vector_size(&self->compilation.files));

    if (self->compilation.flags.print_lex) {
        fprintf_s(self->compilation.debug, STR("LEX:\n---\n"));
    }
    Result(Vector(Token), ParserError) lex = silis_parser_lex((silis_parser_lex_input) {
        .allocator = allocator,
        .source = file,
    });
    if (lex.kind.val != Result_Ok) {
        ParserError_print(lex.u.Err, self->compilation.debug);
        unreachable(return (Ref(InterpreterFilePtr)) Ref_null);
    }
    Vector(Token) tokens = lex.u.Ok;
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
    InterpreterFile *f = new(InterpreterFile, ((InterpreterFile) {
            .allocator = allocator,
            .path = path,
            .content = Slice_data_mut(&file.bytes),
            .tokens = tokens,
            .nodes = parse.nodes,
            .entry = {.file = ret, .node = parse.root},
    }));
    Vector_push(&self->compilation.files, f);

    return ret;
}

void Interpreter_eval(Interpreter *self, Ref(InterpreterFilePtr) file)
{
    Allocator *allocator = self->allocator;
    const InterpreterFile *f = Interpreter_lookup_file(self, file);
    FilePath dir = FilePath_dirname(f->path, allocator);
    fs_dirtoken state = fs_pushd(dir, allocator);
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
