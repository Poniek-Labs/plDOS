#include "kernel_connector.h"

int prog_run(int argc, char **argv) {
    char code[KC_FS_MAX_FILE_SIZE + 1];
    int read;
    int rc = 0;

    if (argc < 2) {
        kc_println("Usage: run <program.pbc>");
        return 1;
    }

    read = kc_fs_read(argv[1], (unsigned char *)code, KC_FS_MAX_FILE_SIZE);
    if (read < 0) {
        kc_println("Program not found");
        return 1;
    }
    code[read] = '\0';

    if (!(code[0] == 'P' && code[1] == 'L' && code[2] == 'B' && code[3] == 'C' && code[4] == '1')) {
        kc_println("Not a plDOS bytecode program");
        return 1;
    }

    for (int i = 0; code[i] != '\0'; i++) {
        if (code[i] == 'P' && code[i + 1] == ':') {
            i += 2;
            while (code[i] != '\0' && code[i] != '\n') {
                char out[2];
                out[0] = code[i++];
                out[1] = '\0';
                kc_print(out);
            }
            kc_println("");
        } else if (code[i] == 'R' && code[i + 1] == ':') {
            if (code[i + 2] >= '0' && code[i + 2] <= '9') {
                rc = code[i + 2] - '0';
            }
        }
    }

    return rc;
}
