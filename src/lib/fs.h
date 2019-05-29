#pragma once

#include "allocator.h"
#include "buffer.h"
#include "string.h"
#include "vector.h"

typedef struct {
    String _data;
    Vector(String) parts;
    bool absolute;
    PADDING(7)
} FilePath;

void FilePath_delete(FilePath *self);

FilePath FilePath_from(String path, Allocator *allocator);

#define FilePath_from_native(path, allocator) FilePath_from_native_(path, !TARGET_OS_WIN, allocator)

FilePath FilePath_from_native_(String path, bool nix, Allocator *allocator);

#define FilePath_to_native(path, buf, allocator) FilePath_to_native_(path, buf, !TARGET_OS_WIN, allocator)

String FilePath_to_native_(FilePath path, Buffer *buf, bool nix, Allocator *allocator);

FilePath FilePath_dirname(FilePath self, Allocator *allocator);

typedef struct {
    FilePath root;
} FileSystem;

typedef struct File File;

void FileSystem_delete(FileSystem *self);

FileSystem FileSystem_new(FilePath root);

File *FileSystem_open(FileSystem *fs, FilePath path, String mode, Allocator *allocator);

typedef struct {
    ssize_t (*read)(void *self, Slice(uint8_t) out);

    ssize_t (*write)(void *self, Slice(uint8_t) in);

    ssize_t (*seek)(void *self, off64_t pos, uint8_t whence);

    ssize_t (*tell)(void *self);

    ssize_t (*flush)(void *self);

    ssize_t (*close)(void *self);
} File_class;

struct File {
    Allocator *allocator;
    File_class class;
    void *self;
};

File *File_new(File_class class, void *self, Allocator *allocator);

ssize_t File_read(File *self, Slice(uint8_t) out);

ssize_t File_write(File *self, Slice(uint8_t) in);

enum {
    File_seek_invalid,
    File_seek_begin,
    File_seek_end,
};

ssize_t File_seek(File *self, off64_t pos, uint8_t whence);

ssize_t File_tell(File *self);

ssize_t File_flush(File *self);

ssize_t File_close(File *self);

bool File_read_all(File *self, Slice(uint8_t) *out, Allocator *allocator);

typedef struct {
    Allocator *allocator;
    native_char_t *prev;
} fs_dirtoken;

fs_dirtoken fs_pushd(FilePath path, Allocator *allocator);

void fs_popd(fs_dirtoken tok);

File *fs_stdout(Allocator *allocator);
