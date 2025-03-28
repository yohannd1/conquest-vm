use crate::byte::{Instruction, Register};

pub struct Vm {
    ram: Vec<u8>,
    registers: [u32; 16],
}

enum VmResponse {
    End,
    Continue,
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
        loop {
            match self.process_instruction() {
                Err(e) => return Err(e),
                Ok(VmResponse::End) => return Ok(()),
                Ok(VmResponse::Continue) => {},
            }
        }
    }

    fn process_instruction(&mut self) -> Result<VmResponse, String> {
        fn get_2reg(s: &mut Vm) -> Result<(u8, u8), String> {
            let byte = s.get_next_byte()?;
            let reg1 = (byte & 0b11100000) >> 5;
            let reg2 = (byte & 0b00011100) >> 2;
            Ok((reg1, reg2))
        }

        let byte = self.get_next_byte()?;

        if byte == Instruction::Brk as u8 {
            return Ok(VmResponse::End);
        }

        if byte == Instruction::Cpy as u8 {
            let (rdest, rsrc) = get_2reg(self)?;
            println!("cpy r{rdest:x} <- r{rsrc:x}");
            self.registers[rdest as usize] = self.registers[rsrc as usize];

            return Ok(VmResponse::Continue);
        }

        if byte == Instruction::JmpIf as u8 {
            todo!();
        } else if byte == Instruction::Ld8 as u8 {
            todo!();
        } else if byte == Instruction::Ld16 as u8 {
            todo!();
        }

        if byte == Instruction::Ld32 as u8 {
            let (rdest, _) = get_2reg(self)?;

            let mut data_bytes: [u8; 4] = [0; 4];
            for i in 0..4 {
                data_bytes[i] = self.get_next_byte()?;
            }
            let value = u32::from_be_bytes(data_bytes);

            println!("ld32 r{rdest:X} <- {value} (#{value:x})");
            self.registers[rdest as usize] = value;

            return Ok(VmResponse::Continue);
        }

        if byte == Instruction::Rd8 as u8 {
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
        }

        Err(format!("unknown instruction {byte:02X}"))
    }

    pub fn get_next_byte(&mut self) -> Result<u8, String> {
        let idx = self.registers[Register::InsPtr as usize] as usize;
        if idx >= self.ram.len() {
            Err("instruction pointer past end of memory".into())
        } else {
            let val = self.ram[idx];
            self.registers[Register::InsPtr as usize] += 1;
            Ok(val)
        }
    }
}
