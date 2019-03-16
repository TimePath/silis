#pragma once

#include "string.h"

typedef struct {
    ssize_t (*read)(void *self, Slice(uint8_t) out);

    ssize_t (*write)(void *self, Slice(uint8_t) in);

    ssize_t (*seek)(void *self, off64_t pos, uint8_t whence);

    ssize_t (*tell)(void *self);

    ssize_t (*flush)(void *self);

    ssize_t (*close)(void *self);
} File_class;

typedef struct {
    File_class class;
    void *self;
} File;

File *fs_open_(File_class class, void *self);

File *fs_stdout(void);

File *fs_open(String path, String mode);

ssize_t fs_read(File *self, Slice(uint8_t) out);

ssize_t fs_write(File *self, Slice(uint8_t) in);

ssize_t fs_seek(File *self, off64_t pos, uint8_t whence);

ssize_t fs_tell(File *self);

ssize_t fs_flush(File *self);

ssize_t fs_close(File *self);

uint8_t *fs_read_all(String path, String *out);

typedef struct {
    native_char_t *prev;
} fs_dirtoken;

fs_dirtoken fs_pushd_dirname(String path);

fs_dirtoken fs_pushd(String path);

void fs_popd(fs_dirtoken tok);
