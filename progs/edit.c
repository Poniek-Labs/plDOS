#include "kernel_connector.h"

static void read_line(char *buffer, int max_len);

int prog_edit(int argc, char **argv) {
    char contents[KC_FS_MAX_FILE_SIZE];
    char line[256];
    int used = 0;

    if (argc < 2) {
        kc_println("Usage: edit <file>");
        return 1;
    }

    kc_print("Editing ");
    kc_println(argv[1]);
    kc_println("Enter text. A single dot on a line saves and exits.");
    kc_println("");

    int existing = kc_fs_read(argv[1], (unsigned char *)contents, KC_FS_MAX_FILE_SIZE - 1);
    if (existing > 0) {
        contents[existing] = '\0';
        kc_println("Current contents:");
        kc_println(contents);
        kc_println("---");
    }

    contents[0] = '\0';
    while (used + 2 < KC_FS_MAX_FILE_SIZE) {
        kc_print("> ");
        read_line(line, sizeof(line));
        if (kc_strcmp(line, ".") == 0) {
            break;
        }

        int len = (int)kc_strlen(line);
        if (used + len + 1 >= KC_FS_MAX_FILE_SIZE) {
            kc_println("Editor buffer full");
            break;
        }
        kc_strcat(contents, line);
        kc_strcat(contents, "\n");
        used += len + 1;
    }

    if (!kc_fs_exists(argv[1])) {
        int rc = kc_fs_touch(argv[1]);
        if (rc != 0) {
            return rc;
        }
    }

    return kc_fs_write(argv[1], (const unsigned char *)contents, (uint32_t)kc_strlen(contents));
}

static void read_line(char *buffer, int max_len) {
    int len = 0;
    for (;;) {
        char c = kc_read_char();
        if (c == '\n') {
            kc_println("");
            break;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
                kc_print("\b");
            }
            continue;
        }
        if (len + 1 < max_len) {
            char out[2];
            buffer[len++] = c;
            out[0] = c;
            out[1] = '\0';
            kc_print(out);
        }
    }
    buffer[len] = '\0';
}
