/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "kernel.h"
#include "../progs/programs.h"

static void shell_run(void);
static int shell_parse(char *line, char **argv, int max_args);

void kernel_init(void) {
    io_init();
    memory_init();
    users_init();
    disk_init();
    fs_init();
    programs_init();
}

void kernel_panic(const char *message) {
    io_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    io_println("KERNEL PANIC");
    io_println(message);
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

void kmain(unsigned int magic, unsigned int multiboot_info) {
    (void)magic;
    (void)multiboot_info;

    kernel_init();
    io_clear_screen();
    io_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    io_println("plDOS 0.1.0");
    io_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    io_println("Terminal-first DOS-style OS. Type 'help' to begin.");
    io_println("");

    shell_run();
}

static void shell_prompt(void) {
    io_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    io_print(users_current_name());
    io_print("@");
    io_print(fs_get_current_dir());
    io_print("> ");
    io_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void shell_run(void) {
    char line[256];
    char *argv[16];

    for (;;) {
        shell_prompt();
        io_read_line(line, sizeof(line));
        str_trim(line);

        if (line[0] == '\0') {
            continue;
        }

        int argc = shell_parse(line, argv, 16);
        if (argc == 0) {
            continue;
        }

        const program_t *program = programs_find(argv[0]);
        if (!program) {
            io_print("Unknown command: ");
            io_println(argv[0]);
            continue;
        }

        int rc = program->main(argc, argv);
        if (rc != 0) {
            io_print("Command returned ");
            io_print_int(rc);
            io_println("");
        }
    }
}

static int shell_parse(char *line, char **argv, int max_args) {
    int argc = 0;
    char *cursor = line;

    while (*cursor != '\0' && argc < max_args) {
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }

        argv[argc++] = cursor;

        if (*cursor == '"') {
            argv[argc - 1] = cursor + 1;
            cursor++;
            while (*cursor != '\0' && *cursor != '"') {
                cursor++;
            }
        } else {
            while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
                cursor++;
            }
        }

        if (*cursor != '\0') {
            *cursor = '\0';
            cursor++;
        }
    }

    return argc;
}
