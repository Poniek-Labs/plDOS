/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef PLDOS_KERNEL_CONNECTOR_H
#define PLDOS_KERNEL_CONNECTOR_H

#include <stddef.h>
#include <stdint.h>

#define KC_OS_NAME "plDOS"
#define KC_OS_VERSION "0.1.0"
#define KC_FS_MAX_FILE_SIZE 4096
#define KC_USER_MAX_NAME 24

typedef struct {
    char name[KC_USER_MAX_NAME];
    int active;
    int admin;
} kc_user_t;

void kc_clear(void);
void kc_print(const char *text);
void kc_println(const char *text);
void kc_print_int(int value);
char kc_read_char(void);

size_t kc_strlen(const char *text);
void kc_strcpy(char *dest, const char *src);
void kc_strcat(char *dest, const char *src);
int kc_strcmp(const char *a, const char *b);

const char *kc_fs_current_dir(void);
int kc_fs_cd(const char *path);
int kc_fs_list(const char *path);
int kc_fs_mkdir(const char *path);
int kc_fs_touch(const char *path);
int kc_fs_delete(const char *path);
int kc_fs_move(const char *src, const char *dest);
int kc_fs_copy(const char *src, const char *dest);
int kc_fs_write(const char *path, const unsigned char *data, uint32_t size);
int kc_fs_read(const char *path, unsigned char *buffer, uint32_t max_size);
int kc_fs_exists(const char *path);
int kc_fs_disk_available(void);
int kc_fs_disk_enabled(void);
int kc_fs_disk_format(void);
int kc_fs_disk_save(void);
int kc_fs_disk_load(void);

const char *kc_disk_status(void);

int kc_users_add(const char *name, int admin);
int kc_users_remove(const char *name);
int kc_users_login(const char *name);
int kc_users_logout(void);
const char *kc_users_current_name(void);
int kc_users_current_is_admin(void);
int kc_users_count(void);
int kc_users_get(int index, kc_user_t *out);

#endif
