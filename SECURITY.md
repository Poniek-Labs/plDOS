# Security Policy

plDOS is an educational hobby OS and is not suitable for storing sensitive data.

## Supported Versions

Only the current `main` branch is supported.

## Reporting Issues

Please open a GitHub issue for security-relevant problems. Include:

- What happened
- How to reproduce it
- Whether it affects QEMU, real hardware, or both
- Any disk or persistence risk

## Safety Warning

The persistence layer writes a raw plDOS storage area to the primary ATA disk.
Do not run it on a disk containing important data.
