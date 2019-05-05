#pragma once

#include "allocator.h"
#include "buffer.h"
#include "string.h"
#include "vector.h"

typedef struct {
    ssize_t (*read)(void *self, Slice(uint8_t) out);

    ssize_t (*write)(void *self, Slice(uint8_t) in);

    ssize_t (*seek)(void *self, off64_t pos, uint8_t whence);

    ssize_t (*tell)(void *self);

    ssize_t (*flush)(void *self);

    ssize_t (*close)(void *self);
} File_class;

typedef struct File {
    Allocator *allocator;
    File_class class;
    void *self;
} File;

typedef struct {
    String _data;
    Vector(String) parts;
    bool absolute;
    PADDING(7)
} FilePath;

void FilePath_delete(FilePath *self);

typedef struct {
    FilePath root;
} FileSystem;

void FileSystem_delete(FileSystem *self);

FileSystem fs_root(FilePath root);

FilePath fs_dirname(Allocator *allocator, FilePath self);

FilePath fs_path_from(Allocator *allocator, String path);

#define fs_path_from_native(allocator, path) fs_path_from_native_(allocator, path, !TARGET_OS_WIN)

FilePath fs_path_from_native_(Allocator *allocator, String path, bool nix);

#define fs_path_to_native(allocator, path, buf) fs_path_to_native_(allocator, path, buf, !TARGET_OS_WIN)

String fs_path_to_native_(Allocator *allocator, FilePath path, Buffer *buf, bool nix);

File *fs_open_(Allocator *allocator, File_class class, void *self);

File *fs_stdout(Allocator *allocator);

File *fs_open(Allocator *allocator, FileSystem *fs, FilePath path, String mode);

ssize_t fs_read(File *self, Slice(uint8_t) out);

ssize_t fs_write(File *self, Slice(uint8_t) in);

ssize_t fs_seek(File *self, off64_t pos, uint8_t whence);

ssize_t fs_tell(File *self);

ssize_t fs_flush(File *self);

ssize_t fs_close(File *self);

uint8_t *fs_read_all(Allocator *allocator, FileSystem *fs, FilePath path, String *out);

typedef struct {
    Allocator *allocator;
    native_char_t *prev;
} fs_dirtoken;

fs_dirtoken fs_pushd(Allocator *allocator, FilePath path);

void fs_popd(fs_dirtoken tok);
