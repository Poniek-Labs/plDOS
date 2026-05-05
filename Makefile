# Copyright (c) 2026, OpenDOS Project. Based on plDOS.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# plDOS bootable ISO build
AS = nasm
CC = gcc
LD = ld

BUILD_DIR = build
ISO_DIR = iso
BOOT_DIR = boot
KERNEL_DIR = kernel
PROG_DIR = progs
BIN_DIR = bin

CFLAGS = -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib \
	-nodefaultlibs -Wall -Wextra -std=gnu99 -I$(KERNEL_DIR) -I$(PROG_DIR)
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

KERNEL_OBJS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/io.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/filesystem.o \
	$(BUILD_DIR)/users.o \
	$(BUILD_DIR)/disk.o \
	$(BUILD_DIR)/kernel_connector.o

PROG_SRCS = $(wildcard $(PROG_DIR)/*.c)
DESKTOP_SRCS = $(wildcard $(PROG_DIR)/desktop/*.c)
PROG_OBJS = $(patsubst $(PROG_DIR)/%.c,$(BUILD_DIR)/prog_%.o,$(PROG_SRCS))
DESKTOP_OBJS = $(patsubst $(PROG_DIR)/desktop/%.c,$(BUILD_DIR)/prog_desktop_%.o,$(DESKTOP_SRCS))
OS_BIN = $(BIN_DIR)/pldos.bin
ISO = $(BIN_DIR)/pldos.iso
DISK = $(BIN_DIR)/pldos.disk

.PHONY: all iso run clean cleandisk rebuild help

all: iso

$(BUILD_DIR) $(BIN_DIR) $(ISO_DIR)/boot/grub:
	mkdir -p $@

$(BUILD_DIR)/boot.o: $(BOOT_DIR)/kernel_entry.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/prog_%.o: $(PROG_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/prog_desktop_%.o: $(PROG_DIR)/desktop/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OS_BIN): $(KERNEL_OBJS) $(PROG_OBJS) $(DESKTOP_OBJS) linker.ld | $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS) $(PROG_OBJS) $(DESKTOP_OBJS)
	@echo "Built kernel: $@"

$(ISO): $(OS_BIN) grub.cfg | $(ISO_DIR)/boot/grub $(BIN_DIR)
	cp $(OS_BIN) $(ISO_DIR)/boot/pldos.bin
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)
	@echo "Built ISO: $@"

$(DISK): | $(BIN_DIR)
	truncate -s 64M $@
	@echo "Created writable data disk: $@"

iso: $(ISO)

run: $(ISO) $(DISK)
	qemu-system-i386 -cdrom $(ISO) -drive file=$(DISK),format=raw,index=0,media=disk

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)/boot
	rm -f $(OS_BIN) $(ISO)
	@echo "Clean complete"

cleandisk:
	rm -f $(DISK)
	@echo "Persistent data disk removed"

rebuild: clean all

help:
	@echo "plDOS build system"
	@echo "  make all      Build bootable ISO"
	@echo "  make iso      Build bootable ISO"
	@echo "  make run      Boot ISO in QEMU"
	@echo "  make clean    Remove build artifacts"
	@echo "  make cleandisk Remove persistent data disk"
	@echo "  make rebuild  Clean and rebuild"
