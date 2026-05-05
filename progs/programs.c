/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "programs.h"
#include "kernel_connector.h"

int prog_about(int argc, char **argv);
int prog_cat(int argc, char **argv);
int prog_cd(int argc, char **argv);
int prog_clear(int argc, char **argv);
int prog_copy(int argc, char **argv);
int prog_del(int argc, char **argv);
int prog_desktop(int argc, char **argv);
int prog_disk(int argc, char **argv);
int prog_echo(int argc, char **argv);
int prog_help(int argc, char **argv);
int prog_initdisk(int argc, char **argv);
int prog_load(int argc, char **argv);
int prog_ls(int argc, char **argv);
int prog_mkdir(int argc, char **argv);
int prog_move(int argc, char **argv);
int prog_pwd(int argc, char **argv);
int prog_touch(int argc, char **argv);
int prog_useradd(int argc, char **argv);
int prog_userdel(int argc, char **argv);
int prog_users(int argc, char **argv);
int prog_login(int argc, char **argv);
int prog_logout(int argc, char **argv);
int prog_save(int argc, char **argv);
int prog_whoami(int argc, char **argv);
int prog_write(int argc, char **argv);

static const program_t programs[] = {
    {"about", "show system information", prog_about, 1},
    {"cat", "view a file", prog_cat, 1},
    {"cd", "change directory", prog_cd, 1},
    {"clear", "clear the terminal", prog_clear, 1},
    {"cls", "clear the terminal", prog_clear, 1},
    {"copy", "copy a file", prog_copy, 1},
    {"cp", "copy a file", prog_copy, 1},
    {"del", "delete a file or empty folder", prog_del, 1},
    {"rm", "delete a file or empty folder", prog_del, 1},
    {"desktop", "developer desktop experiment", prog_desktop, 0},
    {"disk", "show disk persistence status", prog_disk, 1},
    {"echo", "print text", prog_echo, 1},
    {"help", "list commands", prog_help, 1},
    {"initdisk", "initialize persistent storage", prog_initdisk, 1},
    {"load", "load filesystem from disk", prog_load, 1},
    {"ls", "list a directory", prog_ls, 1},
    {"dir", "list a directory", prog_ls, 1},
    {"login", "switch to a user", prog_login, 1},
    {"logout", "return to guest", prog_logout, 1},
    {"mkdir", "create a folder", prog_mkdir, 1},
    {"md", "create a folder", prog_mkdir, 1},
    {"move", "move or rename a file/folder", prog_move, 1},
    {"mv", "move or rename a file/folder", prog_move, 1},
    {"pwd", "print working directory", prog_pwd, 1},
    {"save", "save filesystem to disk", prog_save, 1},
    {"touch", "create an empty file", prog_touch, 1},
    {"mkf", "create an empty file", prog_touch, 1},
    {"useradd", "add a user", prog_useradd, 1},
    {"userdel", "delete a user", prog_userdel, 1},
    {"users", "list users", prog_users, 1},
    {"whoami", "print current user", prog_whoami, 1},
    {"write", "write text to a file", prog_write, 1},
    {"wri", "write text to a file", prog_write, 1},
};

static const int program_count = sizeof(programs) / sizeof(programs[0]);

void programs_init(void) {
    for (int i = 0; i < program_count; i++) {
        if (!programs[i].visible) {
            continue;
        }
        char path[128];
        kc_strcpy(path, "/progs/");
        kc_strcat(path, programs[i].name);
        if (!kc_fs_exists(path)) {
            kc_fs_touch(path);
            kc_fs_write(path, (const unsigned char *)programs[i].help, (uint32_t)kc_strlen(programs[i].help));
        }
    }
}

const program_t *programs_find(const char *name) {
    for (int i = 0; i < program_count; i++) {
        if (kc_strcmp(programs[i].name, name) == 0) {
            return &programs[i];
        }
    }
    return 0;
}

const program_t *programs_all(int *count) {
    *count = program_count;
    return programs;
}
