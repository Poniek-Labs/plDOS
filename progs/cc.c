#include "kernel_connector.h"

static int starts_at(const char *text, int pos, const char *needle);
static int is_digit(char c);
static int parse_return(const char *src);
static int compile_prints(const char *src, char *out);

int prog_cc(int argc, char **argv) {
    char src[KC_FS_MAX_FILE_SIZE + 1];
    char out[KC_FS_MAX_FILE_SIZE];
    int read;
    int rc;

    if (argc < 3) {
        kc_println("Usage: cc <source.c> <output.pbc>");
        kc_println("Tiny syntax: print(\"text\"); and return N;");
        return 1;
    }

    read = kc_fs_read(argv[1], (unsigned char *)src, KC_FS_MAX_FILE_SIZE);
    if (read < 0) {
        kc_println("Source not found");
        return 1;
    }
    src[read] = '\0';

    kc_strcpy(out, "PLBC1\n");
    rc = compile_prints(src, out);
    kc_strcat(out, "R:");
    char ret[4];
    int value = parse_return(src);
    ret[0] = (char)('0' + (value % 10));
    ret[1] = '\n';
    ret[2] = '\0';
    kc_strcat(out, ret);

    if (!kc_fs_exists(argv[2])) {
        kc_fs_touch(argv[2]);
    }
    if (kc_fs_write(argv[2], (const unsigned char *)out, (uint32_t)kc_strlen(out)) != 0) {
        return 1;
    }

    kc_print("Compiled print statements: ");
    kc_print_int(rc);
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
