/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "../include/filesystem.h"
#include "../include/io.h"
#include "../include/string.h"
#include <stdlib.h>
#include <stdio.h>

static filesystem_t *fs = NULL;
static char current_dir[MAX_PATH] = "/";

void fs_init(void) {
    if (fs == NULL) {
        fs = (filesystem_t *)malloc(sizeof(filesystem_t));
        fs->file_count = 0;
        
        // Create root directory
        fs->files[0].type = FILE_TYPE_DIR;
        str_copy(fs->files[0].name, "/");
        fs->files[0].size = 0;
        fs->file_count = 1;
    }
}

void fs_create_file(const char *name) {
    if (fs == NULL) return;
    if (fs->file_count >= MAX_FILES) {
        io_println("Error: Maximum file limit reached");
        return;
    }
    
    if (fs_file_exists(name)) {
        io_println("Error: File already exists");
        return;
    }
    
    file_entry_t *new_file = &fs->files[fs->file_count];
    str_copy(new_file->name, name);
    new_file->type = FILE_TYPE_FILE;
    new_file->size = 0;
    new_file->created = 0;
    fs->file_count++;
    
    io_print("Created file: ");
    io_println(name);
}

void fs_create_dir(const char *name) {
    if (fs == NULL) return;
    if (fs->file_count >= MAX_FILES) {
        io_println("Error: Maximum file limit reached");
        return;
    }
    
    if (fs_file_exists(name)) {
        io_println("Error: Directory already exists");
        return;
    }
    
    file_entry_t *new_dir = &fs->files[fs->file_count];
    str_copy(new_dir->name, name);
    new_dir->type = FILE_TYPE_DIR;
    new_dir->size = 0;
    new_dir->created = 0;
    fs->file_count++;
    
    io_print("Created directory: ");
    io_println(name);
}

int fs_file_exists(const char *name) {
    if (fs == NULL) return 0;
    for (uint16_t i = 0; i < fs->file_count; i++) {
        if (str_compare(fs->files[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

file_entry_t* fs_get_file(const char *name) {
    if (fs == NULL) return NULL;
    for (uint16_t i = 0; i < fs->file_count; i++) {
        if (str_compare(fs->files[i].name, name) == 0) {
            return &fs->files[i];
        }
    }
    return NULL;
}

void fs_list_files(void) {
    if (fs == NULL) return;
    
    io_println("\n=== File Listing ===");
    for (uint16_t i = 0; i < fs->file_count; i++) {
        io_print(fs->files[i].type == FILE_TYPE_DIR ? "[DIR]  " : "[FILE] ");
        io_print(fs->files[i].name);
        io_print(" - Size: ");
        io_print_int(fs->files[i].size);
        io_println(" bytes");
    }
    io_println("====================\n");
}

void fs_write_file(const char *name, const char *data) {
    file_entry_t *file = fs_get_file(name);
    if (file == NULL) {
        io_println("Error: File not found");
        return;
    }
    
    if (file->type == FILE_TYPE_DIR) {
        io_println("Error: Cannot write to directory");
        return;
    }
    
    int data_len = str_len(data);
    if (data_len > FILE_SIZE) {
        io_println("Error: Data too large for file");
        return;
    }
    
    int i = 0;
    while (data[i] != '\0' && i < FILE_SIZE) {
        file->data[i] = data[i];
        i++;
    }
    file->data[i] = '\0';
    file->size = i;
    
    io_println("File written successfully");
}

void fs_read_file(const char *name) {
    file_entry_t *file = fs_get_file(name);
    if (file == NULL) {
        io_println("Error: File not found");
        return;
    }
    
    if (file->type == FILE_TYPE_DIR) {
        io_println("Error: Cannot read directory");
        return;
    }
    
    io_println("\n=== File Contents ===");
    io_println((const char *)file->data);
    io_println("====================\n");
}

void fs_delete_file(const char *name) {
    if (fs == NULL) return;
    
    for (uint16_t i = 0; i < fs->file_count; i++) {
        if (str_compare(fs->files[i].name, name) == 0) {
            // Shift all files after this one
            for (uint16_t j = i; j < fs->file_count - 1; j++) {
                fs->files[j] = fs->files[j + 1];
            }
            fs->file_count--;
            io_print("Deleted: ");
            io_println(name);
            return;
        }
    }
    io_println("Error: File not found");
}

void fs_change_dir(const char *name) {
    if (str_compare(name, "/") == 0) {
        str_copy(current_dir, "/");
        io_println("Changed to root directory");
    } else if (fs_file_exists(name)) {
        file_entry_t *dir = fs_get_file(name);
        if (dir->type == FILE_TYPE_DIR) {
            str_copy(current_dir, name);
            io_print("Changed to directory: ");
            io_println(name);
        } else {
            io_println("Error: Not a directory");
        }
    } else {
        io_println("Error: Directory not found");
    }
}
