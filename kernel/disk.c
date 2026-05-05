/*
Copyright (c) 2026, OpenDOS Project. Based on plDOS.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "disk.h"

#define ATA_DATA 0x1F0
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA_LOW 0x1F3
#define ATA_LBA_MID 0x1F4
#define ATA_LBA_HIGH 0x1F5
#define ATA_DRIVE 0x1F6
#define ATA_COMMAND 0x1F7
#define ATA_STATUS 0x1F7

#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

static int has_disk;
static const char *status_text = "not checked";

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned short inw(unsigned short port) {
    unsigned short ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(unsigned short port, unsigned short value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static void ata_delay(void) {
    for (int i = 0; i < 4; i++) {
        inb(ATA_STATUS);
    }
}

static int wait_not_busy(void) {
    for (int i = 0; i < 100000; i++) {
        if ((inb(ATA_STATUS) & ATA_SR_BSY) == 0) {
            return 1;
        }
    }
    return 0;
}

static int wait_drq(void) {
    for (int i = 0; i < 100000; i++) {
        unsigned char status = inb(ATA_STATUS);
        if (status & ATA_SR_ERR) {
            return 0;
        }
        if ((status & ATA_SR_BSY) == 0 && (status & ATA_SR_DRQ)) {
            return 1;
        }
    }
    return 0;
}

void disk_init(void) {
    unsigned char status = inb(ATA_STATUS);
    if (status == 0xFF || status == 0x00) {
        has_disk = 0;
        status_text = "no primary ATA disk";
        return;
    }

    has_disk = 1;
    status_text = "primary ATA disk detected";
}

int disk_present(void) {
    return has_disk;
}

const char *disk_status(void) {
    return status_text;
}

int disk_read_sector(uint32_t lba, unsigned char *buffer) {
    if (!has_disk || !wait_not_busy()) {
        return 0;
    }

    outb(ATA_DRIVE, (unsigned char)(0xE0 | ((lba >> 24) & 0x0F)));
    outb(ATA_SECTOR_COUNT, 1);
    outb(ATA_LBA_LOW, (unsigned char)(lba & 0xFF));
    outb(ATA_LBA_MID, (unsigned char)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HIGH, (unsigned char)((lba >> 16) & 0xFF));
    outb(ATA_COMMAND, ATA_CMD_READ);

    if (!wait_drq()) {
        return 0;
    }

    for (int i = 0; i < 256; i++) {
        unsigned short word = inw(ATA_DATA);
        buffer[i * 2] = (unsigned char)(word & 0xFF);
        buffer[i * 2 + 1] = (unsigned char)((word >> 8) & 0xFF);
    }
    ata_delay();
    return 1;
}

int disk_write_sector(uint32_t lba, const unsigned char *buffer) {
    if (!has_disk || !wait_not_busy()) {
        return 0;
    }

    outb(ATA_DRIVE, (unsigned char)(0xE0 | ((lba >> 24) & 0x0F)));
    outb(ATA_SECTOR_COUNT, 1);
    outb(ATA_LBA_LOW, (unsigned char)(lba & 0xFF));
    outb(ATA_LBA_MID, (unsigned char)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HIGH, (unsigned char)((lba >> 16) & 0xFF));
    outb(ATA_COMMAND, ATA_CMD_WRITE);

    if (!wait_drq()) {
        return 0;
    }

    for (int i = 0; i < 256; i++) {
        unsigned short word = buffer[i * 2] | ((unsigned short)buffer[i * 2 + 1] << 8);
        outw(ATA_DATA, word);
    }

    outb(ATA_COMMAND, 0xE7);
    wait_not_busy();
    ata_delay();
    return 1;
}
