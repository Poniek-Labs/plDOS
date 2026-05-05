/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t *)0xB8000)
#define SCROLLBACK_LINES 200

typedef struct {
    char ch;
    uint8_t color;
} screen_cell_t;

static screen_cell_t scrollback[SCROLLBACK_LINES][VGA_WIDTH];
static size_t terminal_line;
static size_t terminal_column;
static size_t viewport_top;
static uint8_t terminal_color;
static int extended_scancode;

static uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char ch, uint8_t color) {
    return (uint16_t)ch | (uint16_t)color << 8;
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static size_t latest_view_top(void) {
    if (terminal_line >= VGA_HEIGHT) {
        return terminal_line - VGA_HEIGHT + 1;
    }
    return 0;
}

static void move_cursor(void) {
    size_t cursor_row = 0;
    if (terminal_line >= viewport_top) {
        cursor_row = terminal_line - viewport_top;
        if (cursor_row >= VGA_HEIGHT) {
            cursor_row = VGA_HEIGHT - 1;
        }
    }

    uint16_t pos = (uint16_t)(cursor_row * VGA_WIDTH + terminal_column);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void clear_history_line(size_t line) {
    size_t idx = line % SCROLLBACK_LINES;
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        scrollback[idx][x].ch = ' ';
        scrollback[idx][x].color = terminal_color;
    }
}

static void render(void) {
    size_t oldest = 0;
    if (terminal_line >= SCROLLBACK_LINES) {
        oldest = terminal_line - SCROLLBACK_LINES + 1;
    }

    if (viewport_top < oldest) {
        viewport_top = oldest;
    }
    if (viewport_top > latest_view_top()) {
        viewport_top = latest_view_top();
    }

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        size_t line = viewport_top + y;
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            if (line >= oldest && line <= terminal_line) {
                screen_cell_t cell = scrollback[line % SCROLLBACK_LINES][x];
                VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry(cell.ch, cell.color);
            } else {
                VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
            }
        }
    }

    move_cursor();
}

static void follow_output(void) {
    viewport_top = latest_view_top();
}

void io_init(void) {
    terminal_line = 0;
    terminal_column = 0;
    viewport_top = 0;
    extended_scancode = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t y = 0; y < SCROLLBACK_LINES; y++) {
        clear_history_line(y);
    }
    render();
}

void io_set_color(enum vga_color fg, enum vga_color bg) {
    terminal_color = vga_entry_color(fg, bg);
}

void io_clear_screen(void) {
    terminal_line = 0;
    terminal_column = 0;
    viewport_top = 0;
    for (size_t y = 0; y < SCROLLBACK_LINES; y++) {
        clear_history_line(y);
    }
    render();
}

void io_scroll_up(void) {
    size_t oldest = 0;
    if (terminal_line >= SCROLLBACK_LINES) {
        oldest = terminal_line - SCROLLBACK_LINES + 1;
    }
    if (viewport_top > oldest) {
        viewport_top--;
        render();
    }
}

void io_scroll_down(void) {
    size_t latest = latest_view_top();
    if (viewport_top < latest) {
        viewport_top++;
        render();
    }
}

void io_scroll_page_up(void) {
    for (int i = 0; i < VGA_HEIGHT - 1; i++) {
        io_scroll_up();
    }
}

void io_scroll_page_down(void) {
    for (int i = 0; i < VGA_HEIGHT - 1; i++) {
        io_scroll_down();
    }
}

void io_scroll_bottom(void) {
    follow_output();
    render();
}

void io_backspace(void) {
    if (terminal_column == 0 && terminal_line == 0) {
        return;
    }
    if (terminal_column == 0) {
        terminal_line--;
        terminal_column = VGA_WIDTH - 1;
    } else {
        terminal_column--;
    }

    scrollback[terminal_line % SCROLLBACK_LINES][terminal_column].ch = ' ';
    scrollback[terminal_line % SCROLLBACK_LINES][terminal_column].color = terminal_color;
    follow_output();
    render();
}

void io_print_char(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_line++;
        clear_history_line(terminal_line);
        follow_output();
        render();
        return;
    }

    if (c == '\r') {
        terminal_column = 0;
        render();
        return;
    }

    if (c == '\b') {
        io_backspace();
        return;
    }

    scrollback[terminal_line % SCROLLBACK_LINES][terminal_column].ch = c;
    scrollback[terminal_line % SCROLLBACK_LINES][terminal_column].color = terminal_color;
    terminal_column++;

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_line++;
        clear_history_line(terminal_line);
    }

    follow_output();
    render();
}

void io_print(const char *text) {
    for (size_t i = 0; text && text[i] != '\0'; i++) {
        io_print_char(text[i]);
    }
}

void io_println(const char *text) {
    io_print(text);
    io_print_char('\n');
}

void io_print_int(int value) {
    char buf[16];
    int i = 0;
    int negative = value < 0;
    unsigned int n = negative ? (unsigned int)(-value) : (unsigned int)value;

    if (n == 0) {
        io_print_char('0');
        return;
    }
    while (n > 0 && i < 15) {
        buf[i++] = (char)('0' + (n % 10));
        n /= 10;
    }
    if (negative) {
        buf[i++] = '-';
    }
    while (i > 0) {
        io_print_char(buf[--i]);
    }
}

void io_print_hex(uint32_t value) {
    const char *hex = "0123456789ABCDEF";
    io_print("0x");
    for (int i = 7; i >= 0; i--) {
        io_print_char(hex[(value >> (i * 4)) & 0xF]);
    }
}

static char scancode_to_ascii(uint8_t scancode) {
    static const char table[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8',
        '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
        '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
        'm', ',', '.', '/', 0, '*', 0, ' ', 0
    };
    if (scancode < 128) {
        return table[scancode];
    }
    return 0;
}

char io_read_char(void) {
    for (;;) {
        while ((inb(0x64) & 1) == 0) {
        }

        uint8_t scancode = inb(0x60);
        if (scancode == 0xE0) {
            extended_scancode = 1;
            continue;
        }
        if (scancode & 0x80) {
            extended_scancode = 0;
            continue;
        }

        if (extended_scancode) {
            extended_scancode = 0;
            if (scancode == 0x48) {
                io_scroll_up();
            } else if (scancode == 0x50) {
                io_scroll_down();
            } else if (scancode == 0x49) {
                io_scroll_page_up();
            } else if (scancode == 0x51) {
                io_scroll_page_down();
            } else if (scancode == 0x47) {
                io_scroll_bottom();
            }
            continue;
        }

        char c = scancode_to_ascii(scancode);
        if (c) {
            return c;
        }
    }
}

void io_read_line(char *buffer, size_t max_len) {
    size_t len = 0;
    if (max_len == 0) {
        return;
    }

    for (;;) {
        char c = io_read_char();
        if (c == '\n') {
            io_print_char('\n');
            break;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
                io_backspace();
            }
            continue;
        }
        if (len + 1 < max_len) {
            buffer[len++] = c;
            io_print_char(c);
        }
    }
    buffer[len] = '\0';
}
