# Conquest PC Standard

## Init

At the beginning, the first 64kb are read into memory, and the instruction pointer is set to 0x100 (256). Very similar to Uxn.

## Processor

TODO: registers and their meanings
  TODO: probably upgrade to 16 registers instead of 8

TODO: 0xAABBCCDD at @00 means that @00=AA, @01=BB, @02=CC, @03=DD (is
this little or big endian?)

## Ports/Devices

TODO: properly think about this

```
Group "SYSTEM" (00-128)
  00-01 ERRNO
  02-05 MAX_MEMORY (read and write the amount of memory free for the system. Default's 64000, or 64kb, and the max memory that can be requested depends on the implementation)
```

## Graphics

## Audio
