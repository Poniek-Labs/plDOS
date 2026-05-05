/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

#define MAX_FILENAME 32
#define MAX_PATH 128
#define MAX_FILES 256
#define FILE_SIZE 4096

typedef enum {
    FILE_TYPE_FILE = 0,
    FILE_TYPE_DIR = 1
} file_type_t;

typedef struct {
    char name[MAX_FILENAME];
    file_type_t type;
    uint32_t size;
    uint8_t data[FILE_SIZE];
    uint16_t created;
} file_entry_t;

typedef struct {
    file_entry_t files[MAX_FILES];
    uint16_t file_count;
} filesystem_t;

// Filesystem functions
void fs_init(void);
void fs_create_file(const char *name);
void fs_create_dir(const char *name);
int fs_file_exists(const char *name);
file_entry_t* fs_get_file(const char *name);
void fs_list_files(void);
void fs_write_file(const char *name, const char *data);
void fs_read_file(const char *name);
void fs_delete_file(const char *name);
void fs_change_dir(const char *name);

#endif // FILESYSTEM_H
