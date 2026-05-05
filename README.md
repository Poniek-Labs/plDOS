# plDOS

plDOS is a small terminal-first DOS-style operating system written in C. It
builds into a real GRUB multiboot ISO, boots a 32-bit freestanding kernel, and
provides a DOS-inspired shell with files, folders, users, command modules, and
raw ATA disk persistence.

The project is educational and experimental. It is designed to be readable,
hackable, and honest about what is implemented versus what is still future OS
work.

## Current Status

plDOS currently supports:

- Bootable ISO via GRUB multiboot
- 32-bit freestanding C kernel
- VGA text terminal with scrollback
- PS/2 keyboard input
- DOS-style shell
- `/progs` command registry
- Kernel connector API between commands and private kernel services
- In-memory filesystem with file and folder commands
- Auto-save persistence to an attached primary ATA disk
- Basic user commands: `users`, `whoami`, `login`, `logout`, `useradd`, `userdel`
- QEMU run target with a persistent raw data disk

Not implemented yet:

- Protected user processes
- CPU privilege ring isolation
- Paging and virtual memory
- Dynamic ELF/program loading from disk
- AHCI/SATA or USB storage drivers
- Partition-aware installer
- FAT/ext/NTFS filesystem support

## Quick Start

Build and run in QEMU:

```sh
./build.sh
```

Build the ISO without launching QEMU:

```sh
./build.sh iso
```

Generated outputs:

```txt
bin/pldos.iso   bootable ISO
bin/pldos.bin   multiboot kernel binary
bin/pldos.disk  writable QEMU data disk
```

## Requirements

- `gcc`
- `ld`
- `nasm`
- `make`
- `grub-mkrescue`
- `xorriso`
- `qemu-system-i386` for emulator runs

## Shell Commands

Common commands inside plDOS:

```txt
help    about   clear   ls      cd      pwd
touch   mkdir   write   cat     copy    move
del     disk    save    load    users   whoami
login   logout  useradd userdel
```

Aliases include `cls`, `dir`, `mkf`, `md`, `wri`, `cp`, `mv`, and `rm`.

## Persistence Warning

plDOS auto-saves filesystem changes when an ATA disk is attached.

In QEMU, persistence writes to:

```txt
bin/pldos.disk
```

On real hardware, plDOS writes a raw storage area to the primary ATA disk. It is
not partition-aware. Do not run persistence on a disk containing important data.
Use QEMU or a spare disk.

## Documentation

- [Documentation index](docs/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Command reference](docs/COMMANDS.md)
- [**Code Explanation (Line-by-Line)**](docs/CODE_EXPLANATION.md) - Detailed walkthrough of every major code component
- [Kernel connector](docs/KERNEL_CONNECTOR.md)
- [Filesystem and persistence](docs/FILESYSTEM.md)
- [Old laptop guide](docs/OLD_LAPTOP.md)
- [Contributing](docs/CONTRIBUTING.md)

## Repository Layout

```txt
boot/       multiboot entry code
kernel/     private kernel services
progs/      command modules and public connector header
docs/       project documentation
bin/        generated build outputs
iso/        temporary ISO staging tree
```

## License

MIT. See [LICENSE](LICENSE).
