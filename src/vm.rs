use crate::byte::{Register, Instruction};

pub struct Vm {
    ram: Vec<u8>,
    registers: [u32; 16],
}

impl Vm {
    pub fn new(ram_size: usize) -> Self {
        let mut registers = [0; 16];
        registers[Register::InsPtr as usize] = 0x100;

        return Self {
            ram: vec![0; ram_size],
            registers,
        };
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

    pub fn run(&mut self) -> Result<(), String> {
        let byte = self
            .get_next_byte()
            .ok_or_else(|| format!("instruction pointer past end of memory"))?;

        if byte == Instruction::Brk as u8 {
            return Ok(());
        } else if byte == Instruction::Cpy as u8 {
            todo!();
        } else if byte == Instruction::JmpIf as u8 {
            todo!();
        } else if byte == Instruction::Ld8 as u8 {
            todo!();
        } else if byte == Instruction::Ld16 as u8 {
            todo!();
        } else if byte == Instruction::Ld32 as u8 {
            todo!();
        } else if byte == Instruction::Rd8 as u8 {
            todo!();
        } else if byte == Instruction::Rd16 as u8 {
            todo!();
        } else if byte == Instruction::Rd32 as u8 {
            todo!();
        } else if byte == Instruction::Wr8 as u8 {
            todo!();
        } else if byte == Instruction::Wr16 as u8 {
            todo!();
        } else if byte == Instruction::Wr32 as u8 {
            todo!();
        } else if byte == Instruction::Add as u8 {
            todo!();
        } else if byte == Instruction::Sub as u8 {
            todo!();
        } else if byte == Instruction::Mul as u8 {
            todo!();
        } else if byte == Instruction::Div as u8 {
            todo!();
        } else if byte == Instruction::Shl as u8 {
            todo!();
        } else if byte == Instruction::Shr as u8 {
            todo!();
        } else if byte == Instruction::Eq as u8 {
            todo!();
        } else if byte == Instruction::Neq as u8 {
            todo!();
        } else if byte == Instruction::Lt as u8 {
            todo!();
        } else if byte == Instruction::Leq as u8 {
            todo!();
        } else if byte == Instruction::Gt as u8 {
            todo!();
        } else if byte == Instruction::Geq as u8 {
            todo!();
        } else if byte == Instruction::Not as u8 {
            todo!();
        } else if byte == Instruction::Print as u8 {
            todo!();
        } else {
            todo!("error message lol");
        }
    }

    pub fn get_next_byte(&mut self) -> Option<u8> {
        let idx = self.registers[Register::InsPtr as usize] as usize;
        if idx >= self.ram.len() {
            None
        } else {
            let val = self.ram[idx];
            self.registers[Register::InsPtr as usize] += 1;
            Some(val)
        }
    }
}
