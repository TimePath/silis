#pragma once

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

typedef struct File_s {
    File_class class;
    void *self;
} File;

typedef struct {
    String _data;
    Vector(String) parts;
    bool absolute;
    uint8_t _padding[7];
} FilePath;

FilePath fs_dirname(FilePath self);

FilePath fs_path_from(String path);

#define fs_path_from_native(path) fs_path_from_native_(path, !TARGET_OS_WIN)

FilePath fs_path_from_native_(String path, bool nix);

#define fs_path_to_native(path, buf) fs_path_to_native_(path, buf, !TARGET_OS_WIN)

String fs_path_to_native_(FilePath path, Buffer *buf, bool nix);

File *fs_open_(File_class class, void *self);

File *fs_stdout(void);

File *fs_open(FilePath path, String mode);

ssize_t fs_read(File *self, Slice(uint8_t) out);

ssize_t fs_write(File *self, Slice(uint8_t) in);

ssize_t fs_seek(File *self, off64_t pos, uint8_t whence);

ssize_t fs_tell(File *self);

ssize_t fs_flush(File *self);

ssize_t fs_close(File *self);

uint8_t *fs_read_all(FilePath path, String *out);

typedef struct {
    native_char_t *prev;
} fs_dirtoken;

fs_dirtoken fs_pushd(FilePath path);

void fs_popd(fs_dirtoken tok);
