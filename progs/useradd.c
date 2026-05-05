/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "kernel_connector.h"

int prog_useradd(int argc, char **argv) {
    int admin = 0;

    if (!kc_users_current_is_admin()) {
        kc_println("Only an admin can add users");
        return 1;
    }
    if (argc < 2) {
        kc_println("Usage: useradd <name> [admin]");
        return 1;
    }
    if (argc > 2 && kc_strcmp(argv[2], "admin") == 0) {
        admin = 1;
    }

    int rc = kc_users_add(argv[1], admin);
    if (rc == 1) {
        kc_println("Invalid user name");
    } else if (rc == 2) {
        kc_println("User already exists");
    } else if (rc == 3) {
        kc_println("User table full");
    }
    return rc == 0 ? 0 : 1;
}
