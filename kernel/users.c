/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "users.h"
#include "string.h"

static user_t users[USER_MAX_COUNT];
static int current_user;

static int find_user(const char *name) {
    for (int i = 0; i < USER_MAX_COUNT; i++) {
        if (users[i].active && str_compare(users[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void users_init(void) {
    for (int i = 0; i < USER_MAX_COUNT; i++) {
        users[i].active = 0;
        users[i].admin = 0;
        users[i].name[0] = '\0';
    }

    users[0].active = 1;
    users[0].admin = 1;
    str_copy(users[0].name, "root");

    users[1].active = 1;
    users[1].admin = 0;
    str_copy(users[1].name, "guest");

    current_user = 0;
}

int users_add(const char *name, int admin) {
    if (!name || name[0] == '\0' || str_len(name) >= USER_MAX_NAME) {
        return 1;
    }
    if (find_user(name) >= 0) {
        return 2;
    }

    for (int i = 0; i < USER_MAX_COUNT; i++) {
        if (!users[i].active) {
            users[i].active = 1;
            users[i].admin = admin ? 1 : 0;
            str_copy(users[i].name, name);
            return 0;
        }
    }

    return 3;
}

int users_remove(const char *name) {
    int idx = find_user(name);
    if (idx < 0 || idx == 0) {
        return 1;
    }
    if (idx == current_user) {
        return 2;
    }

    users[idx].active = 0;
    users[idx].admin = 0;
    users[idx].name[0] = '\0';
    return 0;
}

int users_login(const char *name) {
    int idx = find_user(name);
    if (idx < 0) {
        return 1;
    }
    current_user = idx;
    return 0;
}

int users_logout(void) {
    current_user = 1;
    return 0;
}

const char *users_current_name(void) {
    return users[current_user].name;
}

int users_current_is_admin(void) {
    return users[current_user].admin;
}

int users_count(void) {
    int count = 0;
    for (int i = 0; i < USER_MAX_COUNT; i++) {
        if (users[i].active) {
            count++;
        }
    }
    return count;
}

int users_get(int index, user_t *out) {
    int seen = 0;
    for (int i = 0; i < USER_MAX_COUNT; i++) {
        if (users[i].active) {
            if (seen == index) {
                *out = users[i];
                return 1;
            }
            seen++;
        }
    }
    return 0;
}

int users_exists(const char *name) {
    return find_user(name) >= 0;
}
