/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "../include/commands.h"
#include "../include/io.h"
#include "../include/filesystem.h"
#include "../include/string.h"
#include <stdlib.h>

static int exit_flag = 0;

void cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    io_println("\n========== OpenDOS - Command Help ==========");
    io_println("help           - Show this help message");
    io_println("clear/cls      - Clear the screen");
    io_println("echo [text]    - Print text to console");
    io_println("mkf [name]     - Create a new file");
    io_println("wri [name]     - Write to a file (opens text editor)");
    io_println("folist/ls      - List files in current directory");
    io_println("del [name]     - Delete a file");
    io_println("cat [name]     - Display file contents");
    io_println("mkdir [name]   - Create a new directory");
    io_println("pwd            - Print working directory");
    io_println("cd [dir]       - Change directory");
    io_println("about          - About OpenDOS");
    io_println("exit/quit      - Exit the operating system");
    io_println("============================================\n");
}

void cmd_clear(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_clear_screen();
}

void cmd_echo(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: echo [text]");
        return;
    }
    
    for (int i = 1; i < argc; i++) {
        io_print(argv[i]);
        if (i < argc - 1) io_print(" ");
    }
    io_println("");
}

void cmd_mkf(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: mkf [filename]");
        return;
    }
    fs_create_file(argv[1]);
}

void cmd_wri(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: wri [filename]");
        return;
    }
    
    if (!fs_file_exists(argv[1])) {
        io_println("Error: File does not exist. Create it first with 'mkf'");
        return;
    }
    
    char buffer[FILE_SIZE] = {0};
    io_println("Enter file contents (max 4096 characters). Type 'EOF' on a new line to finish:");
    
    int pos = 0;
    while (pos < FILE_SIZE - 1) {
        char line[256] = {0};
        io_print("> ");
        io_get_string(line, 255);
        
        if (str_compare(line, "EOF") == 0) {
            break;
        }
        
        int line_len = str_len(line);
        if (pos + line_len + 1 < FILE_SIZE) {
            int i = 0;
            while (line[i] != '\0') {
                buffer[pos++] = line[i];
                i++;
            }
            buffer[pos++] = '\n';
        }
    }
    buffer[pos] = '\0';
    
    fs_write_file(argv[1], buffer);
}

void cmd_folist(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fs_list_files();
}

void cmd_del(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: del [filename]");
        return;
    }
    fs_delete_file(argv[1]);
}

void cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: cat [filename]");
        return;
    }
    fs_read_file(argv[1]);
}

void cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: mkdir [dirname]");
        return;
    }
    fs_create_dir(argv[1]);
}

void cmd_pwd(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_println("Current directory: /");
}

void cmd_cd(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: cd [dirname]");
        return;
    }
    fs_change_dir(argv[1]);
}

void cmd_exit(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_println("Shutting down OpenDOS...");
    exit_flag = 1;
}

void cmd_about(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_println("\n========== About OpenDOS ==========");
    io_println("Name: OpenDOS - Open Disk Operating System");
    io_println("Version: 1.0.0");
    io_println("Platform: Modular C-based OS");
    io_println("Type: Command-line Operating System");
    io_println("\nA simple DOS-style terminal operating");
    io_println("system written in C with basic file");
    io_println("management and shell capabilities.");
    io_println("====================================\n");
}
