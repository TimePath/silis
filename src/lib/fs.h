#pragma once

#include "string.h"

typedef struct {
    native_char_t *prev;
} fs_dirtoken;

fs_dirtoken fs_pushd_dirname(String path);

fs_dirtoken fs_pushd(String path);

void fs_popd(fs_dirtoken tok);

bool fs_read(String path, String *out);
