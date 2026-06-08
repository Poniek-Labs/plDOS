#include "kernel_connector.h"

int prog_wall(int argc, char **argv) {
    (void)argc;
    (void)argv;

    kc_println("Kernel wall: enabled");
    kc_println("");
    kc_println("Programs use progs/kernel_connector.h and kc_* requests.");
    kc_println("Private kernel services stay behind kernel/kernel_connector.c.");
    kc_println("");
    kc_print("Current user: ");
    kc_println(kc_users_current_name());
    kc_print("Admin privileges: ");
    kc_println(kc_users_current_is_admin() ? "yes" : "no");
    kc_println("");
    kc_println("Admin-gated connector requests:");
    kc_println("  disk format");
    kc_println("  disk save");
    kc_println("  disk load");
    return 0;
}
