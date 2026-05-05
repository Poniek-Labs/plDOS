/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "memory.h"

#define HEAP_SIZE (128 * 1024)

static unsigned char heap[HEAP_SIZE];
static size_t heap_used;

void memory_init(void) {
    heap_used = 0;
}

void *kmalloc(size_t size) {
    if (size == 0) {
        return 0;
    }

    size = (size + 7) & ~((size_t)7);
    if (heap_used + size > HEAP_SIZE) {
        return 0;
    }

    void *ptr = &heap[heap_used];
    heap_used += size;
    return ptr;
}

void kfree(void *ptr) {
    (void)ptr;
}

void kmemset(void *ptr, int value, size_t size) {
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = (unsigned char)value;
    }
}

void kmemcpy(void *dest, const void *src, size_t size) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

int kmemcmp(const void *a, const void *b, size_t size) {
    const unsigned char *left = (const unsigned char *)a;
    const unsigned char *right = (const unsigned char *)b;
    for (size_t i = 0; i < size; i++) {
        if (left[i] != right[i]) {
            return left[i] - right[i];
        }
    }
    return 0;
}

size_t kmem_get_allocated(void) {
    return heap_used;
}

size_t kmem_get_free(void) {
    return HEAP_SIZE - heap_used;
}
