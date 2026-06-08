#include "kernel_connector.h"

static void ensure_apps_dir(void);
static void path_for(char *out, const char *name, const char *ext);
static int compile_app(const char *src_path, const char *bin_path);
static int starts_at(const char *text, int pos, const char *needle);
static int parse_return(const char *src);
static int compile_prints(const char *src, char *out);
static int is_digit(char c);

int prog_apps(int argc, char **argv) {
    char src_path[96];
    char bin_path[96];

    if (argc < 2) {
        kc_println("Usage: apps <list|new|build|run> [name]");
        return 1;
    }

    ensure_apps_dir();

    if (kc_strcmp(argv[1], "list") == 0) {
        return kc_fs_list("/home/apps");
    }

    if (argc < 3) {
        kc_println("App name required");
        return 1;
    }

    path_for(src_path, argv[2], ".c");
    path_for(bin_path, argv[2], ".pbc");

    if (kc_strcmp(argv[1], "new") == 0) {
        const char *template_src =
            "print(\"hello from app\");\n"
            "return 0;\n";
        if (!kc_fs_exists(src_path)) {
            kc_fs_touch(src_path);
        }
        kc_fs_write(src_path, (const unsigned char *)template_src, (uint32_t)kc_strlen(template_src));
        kc_print("Created ");
        kc_println(src_path);
        kc_println("Edit it with: edit <path>");
        return 0;
    }

    if (kc_strcmp(argv[1], "build") == 0) {
        return compile_app(src_path, bin_path);
    }

    if (kc_strcmp(argv[1], "run") == 0) {
        char *runner_argv[2];
        extern int prog_run(int argc, char **argv);
        runner_argv[0] = "run";
        runner_argv[1] = bin_path;
        return prog_run(2, runner_argv);
    }

    kc_println("Unknown apps command");
    return 1;
}

static void ensure_apps_dir(void) {
    if (!kc_fs_exists("/home/apps")) {
        kc_fs_mkdir("/home/apps");
    }
}

static void path_for(char *out, const char *name, const char *ext) {
    kc_strcpy(out, "/home/apps/");
    kc_strcat(out, name);
    kc_strcat(out, ext);
}

static int compile_app(const char *src_path, const char *bin_path) {
    char src[KC_FS_MAX_FILE_SIZE + 1];
    char out[KC_FS_MAX_FILE_SIZE];
    int read = kc_fs_read(src_path, (unsigned char *)src, KC_FS_MAX_FILE_SIZE);

    if (read < 0) {
        kc_println("Source app not found");
        return 1;
    }
    src[read] = '\0';

    kc_strcpy(out, "PLBC1\n");
    int prints = compile_prints(src, out);
    kc_strcat(out, "R:");
    char ret[4];
    int value = parse_return(src);
    ret[0] = (char)('0' + (value % 10));
    ret[1] = '\n';
    ret[2] = '\0';
    kc_strcat(out, ret);

    if (!kc_fs_exists(bin_path)) {
        kc_fs_touch(bin_path);
    }
    if (kc_fs_write(bin_path, (const unsigned char *)out, (uint32_t)kc_strlen(out)) != 0) {
        return 1;
    }

    kc_print("Built ");
    kc_print(bin_path);
    kc_print(" with print statements: ");
    kc_print_int(prints);
    kc_println("");
    return 0;
}

static int compile_prints(const char *src, char *out) {
    int compiled = 0;
    for (int i = 0; src[i] != '\0'; i++) {
        if (starts_at(src, i, "print")) {
            while (src[i] != '\0' && src[i] != '"') {
                i++;
            }
            if (src[i] == '"') {
                i++;
                kc_strcat(out, "P:");
                while (src[i] != '\0' && src[i] != '"') {
                    char ch[2];
                    ch[0] = src[i++];
                    ch[1] = '\0';
                    kc_strcat(out, ch);
                }
                kc_strcat(out, "\n");
                compiled++;
            }
        }
    }
    return compiled;
}

static int parse_return(const char *src) {
    for (int i = 0; src[i] != '\0'; i++) {
        if (starts_at(src, i, "return")) {
            while (src[i] != '\0' && !is_digit(src[i])) {
                i++;
            }
            if (is_digit(src[i])) {
                return src[i] - '0';
            }
        }
    }
    return 0;
}

static int starts_at(const char *text, int pos, const char *needle) {
    for (int i = 0; needle[i] != '\0'; i++) {
        if (text[pos + i] != needle[i]) {
            return 0;
        }
    }
    return 1;
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}
