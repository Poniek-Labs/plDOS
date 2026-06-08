#include "kernel_connector.h"

static void ensure_repo(void);
static int read_file(const char *path, char *out, int max);
static void append_line(const char *path, const char *line);

int prog_git(int argc, char **argv) {
    char log[KC_FS_MAX_FILE_SIZE + 1];
    char index[KC_FS_MAX_FILE_SIZE + 1];

    if (argc < 2) {
        kc_println("Usage: git <init|status|add|commit|log>");
        return 1;
    }

    if (kc_strcmp(argv[1], "init") == 0) {
        ensure_repo();
        kc_println("Initialized local plDOS git store");
        return 0;
    }

    ensure_repo();

    if (kc_strcmp(argv[1], "status") == 0) {
        kc_println("Tracked files:");
        if (read_file("/home/.git/index", index, sizeof(index)) > 0) {
            kc_println(index);
        } else {
            kc_println("(none)");
        }
        return 0;
    }

    if (kc_strcmp(argv[1], "add") == 0) {
        if (argc < 3) {
            kc_println("Usage: git add <file>");
            return 1;
        }
        append_line("/home/.git/index", argv[2]);
        kc_println("Added file to local index");
        return 0;
    }

    if (kc_strcmp(argv[1], "commit") == 0) {
        if (argc < 3) {
            kc_println("Usage: git commit <message>");
            return 1;
        }
        append_line("/home/.git/log", argv[2]);
        kc_println("Committed local snapshot message");
        return 0;
    }

    if (kc_strcmp(argv[1], "log") == 0) {
        if (read_file("/home/.git/log", log, sizeof(log)) > 0) {
            kc_println(log);
        } else {
            kc_println("(no commits)");
        }
        return 0;
    }

    kc_println("Unknown git command");
    return 1;
}

static void ensure_repo(void) {
    if (!kc_fs_exists("/home/.git")) {
        kc_fs_mkdir("/home/.git");
    }
    if (!kc_fs_exists("/home/.git/index")) {
        kc_fs_touch("/home/.git/index");
    }
    if (!kc_fs_exists("/home/.git/log")) {
        kc_fs_touch("/home/.git/log");
    }
}

static int read_file(const char *path, char *out, int max) {
    int read = kc_fs_read(path, (unsigned char *)out, (uint32_t)(max - 1));
    if (read < 0) {
        out[0] = '\0';
        return 0;
    }
    out[read] = '\0';
    return read;
}

static void append_line(const char *path, const char *line) {
    char data[KC_FS_MAX_FILE_SIZE];
    read_file(path, data, sizeof(data));
    if (kc_strlen(data) + kc_strlen(line) + 2 >= KC_FS_MAX_FILE_SIZE) {
        kc_println("git store full");
        return;
    }
    kc_strcat(data, line);
    kc_strcat(data, "\n");
    kc_fs_write(path, (const unsigned char *)data, (uint32_t)kc_strlen(data));
}
