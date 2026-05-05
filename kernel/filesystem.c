/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "filesystem.h"
#include "io.h"
#include "memory.h"
#include "string.h"
#include "disk.h"

static fs_node_t nodes[FS_MAX_FILES];
static char current_dir[FS_MAX_PATH];
static int persistence_enabled;
static int fs_booting;

#define FS_DISK_MAGIC "PLDOSFS1"
#define FS_DISK_VERSION 1
#define FS_DISK_START_LBA 2048
#define FS_DISK_BYTES ((uint32_t)sizeof(nodes))
#define FS_DISK_DATA_SECTORS ((FS_DISK_BYTES + 511) / 512)

static int allocate_node(void);
static int find_node(const char *path);
static int find_parent(const char *path);
static void rebuild_child_paths(int parent);
static void create_default_tree(void);
static void persist_if_enabled(void);
static void write_u32(unsigned char *buf, uint32_t value);
static uint32_t read_u32(const unsigned char *buf);

void fs_init(void) {
    kmemset(nodes, 0, sizeof(nodes));
    str_copy(current_dir, "/");
    persistence_enabled = 0;
    fs_booting = 1;

    if (disk_present()) {
        if (fs_disk_load() == 0) {
            fs_booting = 0;
            return;
        }

        create_default_tree();
        fs_disk_save();
        fs_booting = 0;
        return;
    }

    create_default_tree();
    fs_booting = 0;
}

const char *fs_get_current_dir(void) {
    return current_dir;
}

void fs_resolve_path(const char *input, char *out, size_t out_size) {
    char raw[FS_MAX_PATH];
    char result[FS_MAX_PATH];
    char components[16][FS_MAX_NAME];
    int depth = 0;

    if (!input || input[0] == '\0') {
        input = current_dir;
    }

    if (input[0] == '/') {
        str_copy_n(raw, input, sizeof(raw));
    } else {
        str_copy_n(raw, current_dir, sizeof(raw));
        if (str_compare(raw, "/") != 0) {
            str_concat(raw, "/");
        }
        str_concat(raw, input);
    }

    size_t i = 0;
    while (raw[i] != '\0') {
        while (raw[i] == '/') {
            i++;
        }
        if (raw[i] == '\0') {
            break;
        }

        char part[FS_MAX_NAME];
        size_t len = 0;
        while (raw[i] != '\0' && raw[i] != '/' && len + 1 < FS_MAX_NAME) {
            part[len++] = raw[i++];
        }
        part[len] = '\0';

        if (str_compare(part, ".") == 0) {
            continue;
        }
        if (str_compare(part, "..") == 0) {
            if (depth > 0) {
                depth--;
            }
            continue;
        }
        if (depth < 16) {
            str_copy(components[depth++], part);
        }
    }

    str_copy(result, "/");
    for (int c = 0; c < depth; c++) {
        if (c > 0) {
            str_concat(result, "/");
        }
        str_concat(result, components[c]);
    }

    str_copy_n(out, result, out_size);
}

int fs_change_dir(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    int idx = find_node(resolved);
    if (idx < 0 || nodes[idx].type != FS_DIR) {
        io_println("Directory not found");
        return 1;
    }

    str_copy(current_dir, resolved);
    return 0;
}

int fs_create_file(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    if (find_node(resolved) >= 0) {
        io_println("File already exists");
        return 1;
    }

    int parent = find_parent(resolved);
    if (parent < 0) {
        io_println("Parent directory not found");
        return 1;
    }

    int idx = allocate_node();
    if (idx < 0) {
        io_println("Filesystem full");
        return 1;
    }

    nodes[idx].used = 1;
    nodes[idx].type = FS_FILE;
    nodes[idx].parent = parent;
    nodes[idx].size = 0;
    str_copy_n(nodes[idx].name, path_get_filename(resolved), FS_MAX_NAME);
    str_copy_n(nodes[idx].path, resolved, FS_MAX_PATH);
    persist_if_enabled();
    return 0;
}

int fs_create_dir(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    if (find_node(resolved) >= 0) {
        io_println("Directory already exists");
        return 1;
    }

    int parent = find_parent(resolved);
    if (parent < 0) {
        io_println("Parent directory not found");
        return 1;
    }

    int idx = allocate_node();
    if (idx < 0) {
        io_println("Filesystem full");
        return 1;
    }

    nodes[idx].used = 1;
    nodes[idx].type = FS_DIR;
    nodes[idx].parent = parent;
    nodes[idx].size = 0;
    str_copy_n(nodes[idx].name, path_get_filename(resolved), FS_MAX_NAME);
    str_copy_n(nodes[idx].path, resolved, FS_MAX_PATH);
    persist_if_enabled();
    return 0;
}

int fs_delete(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    int idx = find_node(resolved);
    if (idx <= 0) {
        io_println("Cannot delete that path");
        return 1;
    }

    if (nodes[idx].type == FS_DIR) {
        for (int i = 0; i < FS_MAX_FILES; i++) {
            if (nodes[i].used && nodes[i].parent == idx) {
                io_println("Directory is not empty");
                return 1;
            }
        }
    }

    nodes[idx].used = 0;
    persist_if_enabled();
    return 0;
}

int fs_move(const char *src, const char *dest) {
    char from[FS_MAX_PATH];
    char to[FS_MAX_PATH];
    fs_resolve_path(src, from, sizeof(from));
    fs_resolve_path(dest, to, sizeof(to));

    int idx = find_node(from);
    if (idx <= 0) {
        io_println("Source not found");
        return 1;
    }
    if (find_node(to) >= 0) {
        io_println("Destination already exists");
        return 1;
    }

    int parent = find_parent(to);
    if (parent < 0) {
        io_println("Destination parent not found");
        return 1;
    }

    nodes[idx].parent = parent;
    str_copy_n(nodes[idx].name, path_get_filename(to), FS_MAX_NAME);
    str_copy_n(nodes[idx].path, to, FS_MAX_PATH);
    if (nodes[idx].type == FS_DIR) {
        rebuild_child_paths(idx);
    }
    persist_if_enabled();
    return 0;
}

int fs_copy(const char *src, const char *dest) {
    char from[FS_MAX_PATH];
    fs_resolve_path(src, from, sizeof(from));

    int src_idx = find_node(from);
    if (src_idx < 0 || nodes[src_idx].type != FS_FILE) {
        io_println("Source file not found");
        return 1;
    }
    if (fs_create_file(dest) != 0) {
        return 1;
    }
    return fs_write_file(dest, nodes[src_idx].data, nodes[src_idx].size);
}

int fs_write_file(const char *path, const unsigned char *data, uint32_t size) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    int idx = find_node(resolved);
    if (idx < 0 || nodes[idx].type != FS_FILE) {
        io_println("File not found");
        return 1;
    }
    if (size > FS_MAX_FILE_SIZE) {
        io_println("File too large");
        return 1;
    }

    kmemcpy(nodes[idx].data, data, size);
    nodes[idx].size = size;
    persist_if_enabled();
    return 0;
}

int fs_append_file(const char *path, const unsigned char *data, uint32_t size) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    int idx = find_node(resolved);
    if (idx < 0 || nodes[idx].type != FS_FILE) {
        io_println("File not found");
        return 1;
    }
    if (nodes[idx].size + size > FS_MAX_FILE_SIZE) {
        io_println("File too large");
        return 1;
    }

    kmemcpy(nodes[idx].data + nodes[idx].size, data, size);
    nodes[idx].size += size;
    persist_if_enabled();
    return 0;
}

int fs_read_file(const char *path, unsigned char *buffer, uint32_t max_size) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    int idx = find_node(resolved);
    if (idx < 0 || nodes[idx].type != FS_FILE) {
        return -1;
    }

    uint32_t count = nodes[idx].size < max_size ? nodes[idx].size : max_size;
    kmemcpy(buffer, nodes[idx].data, count);
    return (int)count;
}

int fs_list_dir(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));

    int dir = find_node(resolved);
    if (dir < 0 || nodes[dir].type != FS_DIR) {
        io_println("Directory not found");
        return 1;
    }

    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (nodes[i].used && nodes[i].parent == dir) {
            io_print(nodes[i].type == FS_DIR ? "<DIR>  " : "       ");
            io_print(nodes[i].name);
            if (nodes[i].type == FS_FILE) {
                io_print("  ");
                io_print_int((int)nodes[i].size);
                io_print(" bytes");
            }
            io_println("");
        }
    }
    return 0;
}

int fs_exists(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));
    return find_node(resolved) >= 0;
}

int fs_is_file(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));
    int idx = find_node(resolved);
    return idx >= 0 && nodes[idx].type == FS_FILE;
}

int fs_is_dir(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));
    int idx = find_node(resolved);
    return idx >= 0 && nodes[idx].type == FS_DIR;
}

uint32_t fs_get_file_size(const char *path) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));
    int idx = find_node(resolved);
    return idx >= 0 ? nodes[idx].size : 0;
}

int fs_get_file_info(const char *path, fs_node_t *out) {
    char resolved[FS_MAX_PATH];
    fs_resolve_path(path, resolved, sizeof(resolved));
    int idx = find_node(resolved);
    if (idx < 0) {
        return 0;
    }
    *out = nodes[idx];
    return 1;
}

int fs_disk_available(void) {
    return disk_present();
}

int fs_disk_enabled(void) {
    return persistence_enabled;
}

int fs_disk_format(void) {
    if (!disk_present()) {
        return 1;
    }
    persistence_enabled = 1;
    return fs_disk_save();
}

int fs_disk_save(void) {
    unsigned char sector[512];
    const unsigned char *raw = (const unsigned char *)nodes;

    if (!disk_present()) {
        return 1;
    }

    kmemset(sector, 0, sizeof(sector));
    for (int i = 0; i < 8; i++) {
        sector[i] = FS_DISK_MAGIC[i];
    }
    write_u32(sector + 8, FS_DISK_VERSION);
    write_u32(sector + 12, FS_DISK_BYTES);
    write_u32(sector + 16, FS_MAX_FILES);
    write_u32(sector + 20, FS_MAX_FILE_SIZE);

    if (!disk_write_sector(FS_DISK_START_LBA, sector)) {
        return 1;
    }

    for (uint32_t s = 0; s < FS_DISK_DATA_SECTORS; s++) {
        kmemset(sector, 0, sizeof(sector));
        uint32_t offset = s * 512;
        uint32_t remaining = FS_DISK_BYTES - offset;
        uint32_t chunk = remaining < 512 ? remaining : 512;
        kmemcpy(sector, raw + offset, chunk);
        if (!disk_write_sector(FS_DISK_START_LBA + 1 + s, sector)) {
            return 1;
        }
    }

    persistence_enabled = 1;
    return 0;
}

int fs_disk_load(void) {
    unsigned char sector[512];
    unsigned char *raw = (unsigned char *)nodes;

    if (!disk_present()) {
        return 1;
    }
    if (!disk_read_sector(FS_DISK_START_LBA, sector)) {
        return 1;
    }
    for (int i = 0; i < 8; i++) {
        if (sector[i] != FS_DISK_MAGIC[i]) {
            return 1;
        }
    }
    if (read_u32(sector + 8) != FS_DISK_VERSION ||
        read_u32(sector + 12) != FS_DISK_BYTES ||
        read_u32(sector + 16) != FS_MAX_FILES ||
        read_u32(sector + 20) != FS_MAX_FILE_SIZE) {
        return 1;
    }

    for (uint32_t s = 0; s < FS_DISK_DATA_SECTORS; s++) {
        if (!disk_read_sector(FS_DISK_START_LBA + 1 + s, sector)) {
            return 1;
        }
        uint32_t offset = s * 512;
        uint32_t remaining = FS_DISK_BYTES - offset;
        uint32_t chunk = remaining < 512 ? remaining : 512;
        kmemcpy(raw + offset, sector, chunk);
    }

    str_copy(current_dir, "/");
    persistence_enabled = 1;
    return 0;
}

static int allocate_node(void) {
    for (int i = 1; i < FS_MAX_FILES; i++) {
        if (!nodes[i].used) {
            return i;
        }
    }
    return -1;
}

static int find_node(const char *path) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (nodes[i].used && str_compare(nodes[i].path, path) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_parent(const char *path) {
    char dirname[FS_MAX_PATH];
    str_copy_n(dirname, path_get_dirname(path), sizeof(dirname));
    int parent = find_node(dirname);
    if (parent >= 0 && nodes[parent].type == FS_DIR) {
        return parent;
    }
    return -1;
}

static void rebuild_child_paths(int parent) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (nodes[i].used && nodes[i].parent == parent) {
            char next[FS_MAX_PATH];
            str_copy(next, nodes[parent].path);
            if (str_compare(next, "/") != 0) {
                str_concat(next, "/");
            }
            str_concat(next, nodes[i].name);
            str_copy_n(nodes[i].path, next, FS_MAX_PATH);
            if (nodes[i].type == FS_DIR) {
                rebuild_child_paths(i);
            }
        }
    }
}

static void create_default_tree(void) {
    nodes[0].used = 1;
    nodes[0].type = FS_DIR;
    nodes[0].parent = -1;
    str_copy(nodes[0].name, "/");
    str_copy(nodes[0].path, "/");

    fs_create_dir("/progs");
    fs_create_dir("/home");
    fs_create_file("/home/readme.txt");
    fs_write_file("/home/readme.txt",
                  (const unsigned char *)"Welcome to plDOS. Commands live in /progs as kernel-linked C modules.\n",
                  70);
}

static void persist_if_enabled(void) {
    if (!fs_booting && persistence_enabled) {
        fs_disk_save();
    }
}

static void write_u32(unsigned char *buf, uint32_t value) {
    buf[0] = (unsigned char)(value & 0xFF);
    buf[1] = (unsigned char)((value >> 8) & 0xFF);
    buf[2] = (unsigned char)((value >> 16) & 0xFF);
    buf[3] = (unsigned char)((value >> 24) & 0xFF);
}

static uint32_t read_u32(const unsigned char *buf) {
    return (uint32_t)buf[0] |
           ((uint32_t)buf[1] << 8) |
           ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[3] << 24);
}
