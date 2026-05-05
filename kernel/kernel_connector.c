/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "kernel.h"
#include "../progs/kernel_connector.h"

typedef enum {
    KC_REQ_CLEAR,
    KC_REQ_PRINT,
    KC_REQ_PRINTLN,
    KC_REQ_PRINT_INT,
    KC_REQ_READ_CHAR,
    KC_REQ_STRLEN,
    KC_REQ_STRCPY,
    KC_REQ_STRCAT,
    KC_REQ_STRCMP,
    KC_REQ_FS_CURRENT_DIR,
    KC_REQ_FS_CD,
    KC_REQ_FS_LIST,
    KC_REQ_FS_MKDIR,
    KC_REQ_FS_TOUCH,
    KC_REQ_FS_DELETE,
    KC_REQ_FS_MOVE,
    KC_REQ_FS_COPY,
    KC_REQ_FS_WRITE,
    KC_REQ_FS_READ,
    KC_REQ_FS_EXISTS,
    KC_REQ_FS_DISK_AVAILABLE,
    KC_REQ_FS_DISK_ENABLED,
    KC_REQ_FS_DISK_FORMAT,
    KC_REQ_FS_DISK_SAVE,
    KC_REQ_FS_DISK_LOAD,
    KC_REQ_DISK_STATUS,
    KC_REQ_USERS_ADD,
    KC_REQ_USERS_REMOVE,
    KC_REQ_USERS_LOGIN,
    KC_REQ_USERS_LOGOUT,
    KC_REQ_USERS_CURRENT_NAME,
    KC_REQ_USERS_CURRENT_IS_ADMIN,
    KC_REQ_USERS_COUNT,
    KC_REQ_USERS_GET
} kc_request_t;

typedef struct { const char *text; } kc_text_args_t;
typedef struct { char *dest; const char *src; } kc_str_args_t;
typedef struct { const char *a; const char *b; } kc_cmp_args_t;
typedef struct { const char *path; } kc_path_args_t;
typedef struct { const char *src; const char *dest; } kc_two_path_args_t;
typedef struct { const char *path; const unsigned char *data; uint32_t size; } kc_write_args_t;
typedef struct { const char *path; unsigned char *buffer; uint32_t max_size; } kc_read_args_t;
typedef struct { const char *name; int admin; } kc_user_add_args_t;
typedef struct { int index; kc_user_t *out; } kc_user_get_args_t;

static uintptr_t kernel_connector_request(kc_request_t request, void *args) {
    switch (request) {
        case KC_REQ_CLEAR:
            io_clear_screen();
            return 0;
        case KC_REQ_PRINT:
            io_print(((kc_text_args_t *)args)->text);
            return 0;
        case KC_REQ_PRINTLN:
            io_println(((kc_text_args_t *)args)->text);
            return 0;
        case KC_REQ_PRINT_INT:
            io_print_int((int)(uintptr_t)args);
            return 0;
        case KC_REQ_READ_CHAR:
            return (uintptr_t)io_read_char();
        case KC_REQ_STRLEN:
            return str_len(((kc_text_args_t *)args)->text);
        case KC_REQ_STRCPY:
            str_copy(((kc_str_args_t *)args)->dest, ((kc_str_args_t *)args)->src);
            return 0;
        case KC_REQ_STRCAT:
            str_concat(((kc_str_args_t *)args)->dest, ((kc_str_args_t *)args)->src);
            return 0;
        case KC_REQ_STRCMP:
            return (uintptr_t)str_compare(((kc_cmp_args_t *)args)->a, ((kc_cmp_args_t *)args)->b);
        case KC_REQ_FS_CURRENT_DIR:
            return (uintptr_t)fs_get_current_dir();
        case KC_REQ_FS_CD:
            return fs_change_dir(((kc_path_args_t *)args)->path);
        case KC_REQ_FS_LIST:
            return fs_list_dir(((kc_path_args_t *)args)->path);
        case KC_REQ_FS_MKDIR:
            return fs_create_dir(((kc_path_args_t *)args)->path);
        case KC_REQ_FS_TOUCH:
            return fs_create_file(((kc_path_args_t *)args)->path);
        case KC_REQ_FS_DELETE:
            return fs_delete(((kc_path_args_t *)args)->path);
        case KC_REQ_FS_MOVE:
            return fs_move(((kc_two_path_args_t *)args)->src, ((kc_two_path_args_t *)args)->dest);
        case KC_REQ_FS_COPY:
            return fs_copy(((kc_two_path_args_t *)args)->src, ((kc_two_path_args_t *)args)->dest);
        case KC_REQ_FS_WRITE:
            return fs_write_file(((kc_write_args_t *)args)->path,
                                 ((kc_write_args_t *)args)->data,
                                 ((kc_write_args_t *)args)->size);
        case KC_REQ_FS_READ:
            return (uintptr_t)fs_read_file(((kc_read_args_t *)args)->path,
                                           ((kc_read_args_t *)args)->buffer,
                                           ((kc_read_args_t *)args)->max_size);
        case KC_REQ_FS_EXISTS:
            return fs_exists(((kc_path_args_t *)args)->path);
        case KC_REQ_FS_DISK_AVAILABLE:
            return fs_disk_available();
        case KC_REQ_FS_DISK_ENABLED:
            return fs_disk_enabled();
        case KC_REQ_FS_DISK_FORMAT:
            return fs_disk_format();
        case KC_REQ_FS_DISK_SAVE:
            return fs_disk_save();
        case KC_REQ_FS_DISK_LOAD:
            return fs_disk_load();
        case KC_REQ_DISK_STATUS:
            return (uintptr_t)disk_status();
        case KC_REQ_USERS_ADD:
            return users_add(((kc_user_add_args_t *)args)->name, ((kc_user_add_args_t *)args)->admin);
        case KC_REQ_USERS_REMOVE:
            return users_remove(((kc_text_args_t *)args)->text);
        case KC_REQ_USERS_LOGIN:
            return users_login(((kc_text_args_t *)args)->text);
        case KC_REQ_USERS_LOGOUT:
            return users_logout();
        case KC_REQ_USERS_CURRENT_NAME:
            return (uintptr_t)users_current_name();
        case KC_REQ_USERS_CURRENT_IS_ADMIN:
            return users_current_is_admin();
        case KC_REQ_USERS_COUNT:
            return users_count();
        case KC_REQ_USERS_GET: {
            user_t kernel_user;
            kc_user_get_args_t *get = (kc_user_get_args_t *)args;
            if (!users_get(get->index, &kernel_user)) {
                return 0;
            }
            str_copy(get->out->name, kernel_user.name);
            get->out->active = kernel_user.active;
            get->out->admin = kernel_user.admin;
            return 1;
        }
    }

    return 0;
}

void kc_clear(void) { kernel_connector_request(KC_REQ_CLEAR, 0); }
void kc_print(const char *text) { kc_text_args_t args = {text}; kernel_connector_request(KC_REQ_PRINT, &args); }
void kc_println(const char *text) { kc_text_args_t args = {text}; kernel_connector_request(KC_REQ_PRINTLN, &args); }
void kc_print_int(int value) { kernel_connector_request(KC_REQ_PRINT_INT, (void *)(uintptr_t)value); }
char kc_read_char(void) { return (char)kernel_connector_request(KC_REQ_READ_CHAR, 0); }

size_t kc_strlen(const char *text) { kc_text_args_t args = {text}; return kernel_connector_request(KC_REQ_STRLEN, &args); }
void kc_strcpy(char *dest, const char *src) { kc_str_args_t args = {dest, src}; kernel_connector_request(KC_REQ_STRCPY, &args); }
void kc_strcat(char *dest, const char *src) { kc_str_args_t args = {dest, src}; kernel_connector_request(KC_REQ_STRCAT, &args); }
int kc_strcmp(const char *a, const char *b) { kc_cmp_args_t args = {a, b}; return (int)kernel_connector_request(KC_REQ_STRCMP, &args); }

const char *kc_fs_current_dir(void) { return (const char *)kernel_connector_request(KC_REQ_FS_CURRENT_DIR, 0); }
int kc_fs_cd(const char *path) { kc_path_args_t args = {path}; return (int)kernel_connector_request(KC_REQ_FS_CD, &args); }
int kc_fs_list(const char *path) { kc_path_args_t args = {path}; return (int)kernel_connector_request(KC_REQ_FS_LIST, &args); }
int kc_fs_mkdir(const char *path) { kc_path_args_t args = {path}; return (int)kernel_connector_request(KC_REQ_FS_MKDIR, &args); }
int kc_fs_touch(const char *path) { kc_path_args_t args = {path}; return (int)kernel_connector_request(KC_REQ_FS_TOUCH, &args); }
int kc_fs_delete(const char *path) { kc_path_args_t args = {path}; return (int)kernel_connector_request(KC_REQ_FS_DELETE, &args); }
int kc_fs_move(const char *src, const char *dest) { kc_two_path_args_t args = {src, dest}; return (int)kernel_connector_request(KC_REQ_FS_MOVE, &args); }
int kc_fs_copy(const char *src, const char *dest) { kc_two_path_args_t args = {src, dest}; return (int)kernel_connector_request(KC_REQ_FS_COPY, &args); }
int kc_fs_write(const char *path, const unsigned char *data, uint32_t size) { kc_write_args_t args = {path, data, size}; return (int)kernel_connector_request(KC_REQ_FS_WRITE, &args); }
int kc_fs_read(const char *path, unsigned char *buffer, uint32_t max_size) { kc_read_args_t args = {path, buffer, max_size}; return (int)kernel_connector_request(KC_REQ_FS_READ, &args); }
int kc_fs_exists(const char *path) { kc_path_args_t args = {path}; return (int)kernel_connector_request(KC_REQ_FS_EXISTS, &args); }
int kc_fs_disk_available(void) { return (int)kernel_connector_request(KC_REQ_FS_DISK_AVAILABLE, 0); }
int kc_fs_disk_enabled(void) { return (int)kernel_connector_request(KC_REQ_FS_DISK_ENABLED, 0); }
int kc_fs_disk_format(void) { return (int)kernel_connector_request(KC_REQ_FS_DISK_FORMAT, 0); }
int kc_fs_disk_save(void) { return (int)kernel_connector_request(KC_REQ_FS_DISK_SAVE, 0); }
int kc_fs_disk_load(void) { return (int)kernel_connector_request(KC_REQ_FS_DISK_LOAD, 0); }

const char *kc_disk_status(void) { return (const char *)kernel_connector_request(KC_REQ_DISK_STATUS, 0); }

int kc_users_add(const char *name, int admin) { kc_user_add_args_t args = {name, admin}; return (int)kernel_connector_request(KC_REQ_USERS_ADD, &args); }
int kc_users_remove(const char *name) { kc_text_args_t args = {name}; return (int)kernel_connector_request(KC_REQ_USERS_REMOVE, &args); }
int kc_users_login(const char *name) { kc_text_args_t args = {name}; return (int)kernel_connector_request(KC_REQ_USERS_LOGIN, &args); }
int kc_users_logout(void) { return (int)kernel_connector_request(KC_REQ_USERS_LOGOUT, 0); }
const char *kc_users_current_name(void) { return (const char *)kernel_connector_request(KC_REQ_USERS_CURRENT_NAME, 0); }
int kc_users_current_is_admin(void) { return (int)kernel_connector_request(KC_REQ_USERS_CURRENT_IS_ADMIN, 0); }
int kc_users_count(void) { return (int)kernel_connector_request(KC_REQ_USERS_COUNT, 0); }
int kc_users_get(int index, kc_user_t *out) { kc_user_get_args_t args = {index, out}; return (int)kernel_connector_request(KC_REQ_USERS_GET, &args); }
