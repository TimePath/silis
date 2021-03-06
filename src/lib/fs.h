#pragma once

#include "allocator.h"
#include "buffer.h"
#include "string.h"
#include "vector.h"

typedef struct File File;
typedef struct FilePath FilePath;
typedef struct FileSystem FileSystem;

struct FilePath {
    String _data;
    Vector(String) parts;
    bool absolute;
    PADDING(7);
};

void FilePath_delete(FilePath *self);

FilePath FilePath_from(String path, Allocator *allocator);

#define FilePath_from_native(path, allocator) FilePath_from_native_(path, TARGET_OS != OS_WINDOWS, allocator)

FilePath FilePath_from_native_(String path, bool nix, Allocator *allocator);

#define FilePath_to_native(path, buf) FilePath_to_native_(path, buf, TARGET_OS != OS_WINDOWS)

String FilePath_to_native_(FilePath path, Buffer *buf, bool nix);

FilePath FilePath_dirname(FilePath self, Allocator *allocator);

FilePath FilePath_basename(FilePath self, Allocator *allocator);

typedef struct {
    File *(*open)(void *self, FilePath path, String mode);
} FileSystem_class;

struct FileSystem {
    Allocator *allocator;
    FileSystem_class class;
    FilePath root;
    void *data;
};

void FileSystem_delete(FileSystem *self);

FileSystem FileSystem_new(FileSystem_class class, void *data, Allocator *allocator);

void FileSystem_newroot(FileSystem *parent, FilePath root, FileSystem *out);

File *FileSystem_open(FileSystem *self, FilePath path, String mode);

typedef struct {
    size_t (*read)(void *self, Slice(uint8_t) out);

    size_t (*write)(void *self, Slice(uint8_t) in);

    size_t (*size)(void *self);

    bool (*rewind)(void *self);

    bool (*flush)(void *self);

    bool (*close)(void *self);
} File_class;

struct File {
    FileSystem *owner;
    Allocator *allocator;
    File_class class;
    void *data;
};

File *File_new(File_class class, void *data, FileSystem *owner, Allocator *allocator);

size_t File_read(File *self, Slice(uint8_t) out);

size_t File_write(File *self, Slice(uint8_t) in);

size_t File_size(File *self);

bool File_rewind(File *self);

bool File_flush(File *self);

bool File_close(File *self);

bool File_read_all(File *self, Slice(uint8_t) *out, Allocator *allocator);

typedef struct {
    Allocator *allocator;
    native_char_t *prev;
} fs_dirtoken;

fs_dirtoken fs_pushd(FilePath path, Allocator *allocator);

void fs_popd(fs_dirtoken tok);
