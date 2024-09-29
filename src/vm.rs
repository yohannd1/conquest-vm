pub struct Vm {
    ram: Vec<u8>,
    registers: [u32; 16],
}

impl Vm {
    pub fn new(ram_size: usize) -> Self {
        let mut registers = [0; 16];
        registers[0x0F] = 0x100;

        return Self {
            ram: vec![0; ram_size],
            registers,
        }
    }

    /// Copies `rom` to the VM's RAM, starting from `start_addr`
    ///
    /// If there is not enough space, the contents will be truncated.
    pub fn load_rom(&mut self, rom: &[u8], start_addr: usize) {
        let dst = &mut self.ram[start_addr..];
        let size = std::cmp::min(dst.len(), rom.len());
        let src = &rom[..size];
        (&mut dst[..src.len()]).copy_from_slice(&src);
    }

    pub fn run(&mut self) {
        unimplemented!("todo...")
    }
}
