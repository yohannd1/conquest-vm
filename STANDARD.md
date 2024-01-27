# Conquest PC Standard

## Hardware specifications

- Processor is big endian (0xAABB = [0xAA, 0xBB])
- Base model has 64KB of memory

## Initialization

The first 1KB is read into memory, and execution starts at 0x0000.

## Processor

TODO: registers and their meanings
  TODO: probably upgrade to 16 registers instead of 8

## Ports/Devices

TODO: properly think about this

```
Group "SYSTEM" (00-128)
  00-01 ERRNO
  02-05 MAX_MEMORY (read and write the amount of memory free for the system. Default's 64000, or 64kb, and the max memory that can be requested depends on the implementation)
```

## Graphics

## Audio
