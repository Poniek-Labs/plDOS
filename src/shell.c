/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "../kernel/kernel.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define COMMAND_BUFFER_SIZE 256
#define ARGS_MAX 16
#define ARG_MAX_LEN 64

// Built-in commands
int cmd_help(int argc, char **argv);
int cmd_clear(int argc, char **argv);
int cmd_echo(int argc, char **argv);
int cmd_pwd(int argc, char **argv);
int cmd_cd(int argc, char **argv);
int cmd_about(int argc, char **argv);
int cmd_ls(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_exit(int argc, char **argv);

typedef struct {
    const char *name;
    int (*func)(int, char **);
    const char *description;
} builtin_command_t;

static builtin_command_t builtins[] = {
    {"help", cmd_help, "Show available commands"},
    {"clear", cmd_clear, "Clear the screen"},
    {"cls", cmd_clear, "Clear the screen (alias)"},
    {"echo", cmd_echo, "Print text to console"},
    {"pwd", cmd_pwd, "Print working directory"},
    {"cd", cmd_cd, "Change directory"},
    {"mkdir", cmd_mkdir, "Create a directory"},
    {"ls", cmd_ls, "List directory contents"},
    {"about", cmd_about, "About OpenDOS"},
    {"exit", cmd_exit, "Exit the system"},
    {"quit", cmd_exit, "Exit the system (alias)"},
    {NULL, NULL, NULL}
};

static int should_exit = 0;

// Built-in command implementations
int cmd_help(int argc, char **argv) {
    io_println("\nBuilt-in commands:");
    for (int i = 0; builtins[i].name != NULL; i++) {
        io_printf("  %-15s - %s\n", builtins[i].name, builtins[i].description);
    }
    io_println("\nYou can also create custom programs in /progs/");
    io_println("and call them like any other command.\n");
    return 0;
}

int cmd_clear(int argc, char **argv) {
    io_clear_screen();
    return 0;
}

int cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        io_print(argv[i]);
        if (i < argc - 1) io_print(" ");
    }
    io_println("");
    return 0;
}

int cmd_pwd(int argc, char **argv) {
    io_println(fs_get_current_dir());
    return 0;
}

int cmd_cd(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: cd <directory>");
        return 1;
    }
    return fs_change_dir(argv[1]);
}

int cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        io_println("Usage: mkdir <directory>");
        return 1;
    }
    return fs_create_dir(argv[1]);
}

int cmd_ls(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : fs_get_current_dir();
    return fs_list_dir(path);
}

int cmd_about(int argc, char **argv) {
    io_println("\n╔═════════════════════════════════════╗");
    io_println("║        OpenDOS 2.0 (Microkernel)   ║");
    io_println("║   A modular, educational DOS OS    ║");
    io_println("║                                     ║");
    io_println("║  Kernel-based architecture with    ║");
    io_println("║  external programs in /progs       ║");
    io_println("╚═════════════════════════════════════╝\n");
    return 0;
}

int cmd_exit(int argc, char **argv) {
    should_exit = 1;
    return 0;
}

// Try to execute a program from /progs
int try_execute_program(const char *cmd, char **argv, int argc) {
    char prog_path[256];
    
    // Build the program path
    str_copy(prog_path, "./bin/");
    str_concat(prog_path, cmd);
    
    // Check if program exists
    if (access(prog_path, X_OK) != 0) {
        return -1;  // Program not found
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute the program
        execv(prog_path, argv);
        exit(1);
    } else if (pid > 0) {
        // Parent process: wait for child
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
    
    return -1;
}

void shell_parse_command(const char *input, char **argv, int *argc) {
    *argc = 1;  // argv[0] is the command name
    int arg_idx = 1;
    char current_arg[ARG_MAX_LEN] = {0};
    int current_len = 0;
    int in_word = 0;
    
    // Initialize argv
    for (int i = 0; i < ARGS_MAX; i++) {
        argv[i] = (char *)malloc(ARG_MAX_LEN);
        if (argv[i]) argv[i][0] = '\0';
    }
    
    // Parse input character by character
    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];
        
        if (c == ' ' || c == '\t' || c == '\n') {
            if (in_word && current_len > 0) {
                current_arg[current_len] = '\0';
                
                if (arg_idx < ARGS_MAX) {
                    str_copy(argv[arg_idx], current_arg);
                    arg_idx++;
                }
                
                current_len = 0;
                in_word = 0;
            }
        } else {
            if (!in_word) {
                in_word = 1;
                current_len = 0;
            }
            if (current_len < ARG_MAX_LEN - 1) {
                current_arg[current_len++] = c;
            }
        }
    }
    
    // Handle final argument
    if (in_word && current_len > 0) {
        current_arg[current_len] = '\0';
        if (arg_idx < ARGS_MAX) {
            str_copy(argv[arg_idx], current_arg);
            arg_idx++;
        }
    }
    
    *argc = arg_idx;
}

void shell_init(void) {
    if (!kernel_init()) {
        io_println("Error: Failed to initialize kernel");
        exit(1);
    }
    
    io_clear_screen();
    io_println("\n");
    io_println("╔═════════════════════════════════════╗");
    io_println("║   Welcome to OpenDOS 2.0 (Kernel)  ║");
    io_println("║  A modular, DOS-style OS in C      ║");
    io_println("║  Type 'help' for available cmds    ║");
    io_println("╚═════════════════════════════════════╝");
    io_println("");
}

void shell_run(void) {
    char command_buffer[COMMAND_BUFFER_SIZE];
    char *argv[ARGS_MAX];
    int argc = 0;
    
    while (!should_exit) {
        io_print("openDOS> ");
        io_get_string(command_buffer, COMMAND_BUFFER_SIZE - 1);
        
        if (str_len(command_buffer) == 0) {
            continue;
        }
        
        shell_parse_command(command_buffer, argv, &argc);
        
        if (argc > 0 && str_len(argv[0]) > 0) {
            const char *cmd = argv[0];
            
            // Check for built-in commands
            int found = 0;
            for (int i = 0; builtins[i].name != NULL; i++) {
                if (str_compare(builtins[i].name, cmd) == 0) {
                    builtins[i].func(argc, argv);
                    found = 1;
                    break;
                }
            }
            
            // Try to execute external program if not found as built-in
            if (!found) {
                int result = try_execute_program(cmd, argv, argc);
                if (result == -1) {
                    io_printf("Error: Unknown command '%s'. Type 'help' for available commands.\n", cmd);
                }
            }
            
            // Free argv
            for (int i = 0; i < ARGS_MAX; i++) {
                if (argv[i]) free(argv[i]);
            }
        }
    }
    
    kernel_shutdown();
}

int main(void) {
    shell_init();
    shell_run();
    return 0;
}
