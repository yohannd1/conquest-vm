# CONQuest

_(yeah, I'm not sure what to name this yet...)_

This is a 32-bit fantasy computer, mostly inspired by Uxn.

Doing this for fun.

## Docs

See the [Standard](STANDARD.md)

## To-Do List

- [x] Initial API
- [x] Implement pointer instructions
- [x] Implement arithmetic instructions
- [ ] Implement device I/O
- [ ] Implement common processor stuff
  - [ ] Timed interrupt (funky way to do multithreading, perhaps? - set
    up an interrupt to run every n cycles - a multiple of 32 i think, so
    i can fit it in an uint32)
  - [ ] Search more about common types of interrupts
- [ ] Graphics (using SDL)
  - [ ] Default rate: 60hz
  - [ ] Related interrupt
- [ ] Audio (using SDL)
