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
- [x] Simplify assembler and interpreter codes (use macros to avoid repetition)
- [ ] Assembler : implement labels and jumps (maybe just a macro to set
  the PC to the specified address... but that might cause an unwanted
  increase of the program counter at the end of the loop)
  - How to track labels and references to jump both backwards and forward?
    - Maybe keep a `Dict<String, Variant<Label, Vector<Addr>>`
    - If we found a jump and the label isn't in the dict, create an
      entry and set the value to the Vector<Addr>
    - If it's in the dict, then check if it's a vector (and add the
      instruction address) or a label (directly insert the label's
      address)
    - If found a label:
      - if it's in the dict as a Label, error (two labels with the same name)
      - if it's in the dict as a Vector<Addr>, replace with the Label
        address and, for each value in the vector, replace with the
        label's address
      - if it's not in the dict, just add the Label entry
    - At the end of it all, if there's any label defs with no address in
      the list, error out with "use of undefined label"
- [ ] Implement device I/O
- [ ] Implement common processor stuff
  - [ ] Timed interrupt (funky way to do multithreading, perhaps? - set
    up an interrupt to run every n cycles - a multiple of 32 i think, so
    i can fit it in an uint32)
  - [ ] Search more about common types of interrupts
  - [ ] Which clock? 4MHz? 8? 16? 32? 64 is probably too much...
- [ ] Implement relative jumps (taking an i8 or i16, I believe!)
- [ ] Relative jump optimizations
- [ ] Graphics (using SDL)
  - [ ] Default rate: 60hz
  - [ ] Related interrupt
- [ ] Audio (using SDL)
  - [ ] Should I put in a dedicated sound chip? and should it already
    exist (e.g. OPL3)?
