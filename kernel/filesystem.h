/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef PLDOS_FILESYSTEM_H
#define PLDOS_FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

#define FS_MAX_NAME 32
#define FS_MAX_PATH 128
#define FS_MAX_FILES 128
#define FS_MAX_FILE_SIZE 4096

typedef enum {
    FS_FILE = 0,
    FS_DIR = 1
} fs_node_type_t;

typedef struct {
    char name[FS_MAX_NAME];
    char path[FS_MAX_PATH];
    fs_node_type_t type;
    int parent;
    uint32_t size;
    unsigned char data[FS_MAX_FILE_SIZE];
    int used;
} fs_node_t;

void fs_init(void);
const char *fs_get_current_dir(void);
int fs_change_dir(const char *path);
int fs_create_file(const char *path);
int fs_create_dir(const char *path);
int fs_delete(const char *path);
int fs_move(const char *src, const char *dest);
int fs_copy(const char *src, const char *dest);
int fs_write_file(const char *path, const unsigned char *data, uint32_t size);
int fs_append_file(const char *path, const unsigned char *data, uint32_t size);
int fs_read_file(const char *path, unsigned char *buffer, uint32_t max_size);
int fs_list_dir(const char *path);
int fs_exists(const char *path);
int fs_is_file(const char *path);
int fs_is_dir(const char *path);
uint32_t fs_get_file_size(const char *path);
int fs_get_file_info(const char *path, fs_node_t *out);
void fs_resolve_path(const char *input, char *out, size_t out_size);
int fs_disk_available(void);
int fs_disk_enabled(void);
int fs_disk_format(void);
int fs_disk_save(void);
int fs_disk_load(void);

#endif
