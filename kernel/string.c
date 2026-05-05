/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "string.h"

size_t str_len(const char *str) {
    size_t len = 0;
    while (str && str[len] != '\0') {
        len++;
    }
    return len;
}

void str_copy(char *dest, const char *src) {
    while (src && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void str_copy_n(char *dest, const char *src, size_t max_len) {
    if (max_len == 0) {
        return;
    }

    size_t i = 0;
    while (src && src[i] != '\0' && i + 1 < max_len) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void str_concat(char *dest, const char *src) {
    size_t i = str_len(dest);
    size_t j = 0;
    while (src && src[j] != '\0') {
        dest[i++] = src[j++];
    }
    dest[i] = '\0';
}

int str_compare(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char *)a - *(const unsigned char *)b;
}

int str_compare_n(const char *a, const char *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i] || a[i] == '\0' || b[i] == '\0') {
            return (unsigned char)a[i] - (unsigned char)b[i];
        }
    }
    return 0;
}

void str_trim(char *str) {
    size_t start = 0;
    size_t end = str_len(str);
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\n') {
        start++;
    }
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\n')) {
        end--;
    }
    size_t out = 0;
    while (start < end) {
        str[out++] = str[start++];
    }
    str[out] = '\0';
}

int str_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str++ != *prefix++) {
            return 0;
        }
    }
    return 1;
}

int str_last_index_of(const char *str, char c) {
    int last = -1;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == c) {
            last = i;
        }
    }
    return last;
}

char *path_get_filename(const char *path) {
    int idx = str_last_index_of(path, '/');
    if (idx < 0) {
        return (char *)path;
    }
    return (char *)(path + idx + 1);
}

char *path_get_dirname(const char *path) {
    static char dirname[128];
    int idx = str_last_index_of(path, '/');
    if (idx <= 0) {
        str_copy(dirname, "/");
        return dirname;
    }
    for (int i = 0; i < idx && i < 127; i++) {
        dirname[i] = path[i];
        dirname[i + 1] = '\0';
    }
    return dirname;
}
