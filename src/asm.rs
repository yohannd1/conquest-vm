use crate::{byte::Instruction, utf8::Utf8CharIterator};

pub fn compile(src: &str) -> Result<Vec<u8>, String> {
    let mut bytecode: Vec<u8> = Vec::new();

    fn extend(bytecode: &mut Vec<u8>, ins: Instruction, slice: &[u8]) {
        bytecode.push(ins as u8);
        bytecode.extend_from_slice(slice);
    }

    fn extend_2reg(
        bytecode: &mut Vec<u8>,
        parser: &mut Parser<'_>,
        ins: Instruction,
    ) -> Result<(), String> {
        let reg1 = parser.read_reg().ok_or_else(|| format!("not a register"))?;
        let reg2 = parser.read_reg().ok_or_else(|| format!("not a register"))?;
        let reg_encoded = (reg1 << 5) + (reg2 << 2);
        extend(bytecode, ins, &[reg_encoded]);
        Ok(())
    }

    let mut p = Parser::new(src);

    while let Some(w) = p.read_word() {
        match w {
            "/*" => loop {
                match p.read_word() {
                    Some("*/") => break,
                    Some(_) => {}
                    None => return Err("unclosed comment".into()),
                }
            },

            "BRK" => bytecode.extend_from_slice(&[Instruction::Brk as u8]), // stop VM

            // copy the value of one register to another
            "CPY" => extend_2reg(&mut bytecode, &mut p, Instruction::Cpy)?,

            // TODO: JMPIF
            // TODO: JMP

            // load (constant into register)
            "LD8" => {
                let reg = p.read_reg().ok_or_else(|| format!("not a register"))?;
                let number = p.read_u32().ok_or_else(|| format!("not a number"))? as u8;

                bytecode.extend_from_slice(&[Instruction::Ld32 as u8, reg, number]);
            }
            "LD16" => {
                let reg = p.read_reg().ok_or_else(|| format!("not a register"))?;
                let number = p.read_u32().ok_or_else(|| format!("not a number"))? as u16;

                bytecode.extend_from_slice(&[Instruction::Ld32 as u8, reg]);
                bytecode.extend_from_slice(&number.to_be_bytes());
            }
            "LD32" => {
                let reg = p.read_reg().ok_or_else(|| format!("not a register"))?;
                let number = p.read_u32().ok_or_else(|| format!("not a number"))?;

                bytecode.extend_from_slice(&[Instruction::Ld32 as u8, reg]);
                bytecode.extend_from_slice(&number.to_be_bytes());
            }

            // read (TODO: rename to fetch?)
            "RD8" => extend_2reg(&mut bytecode, &mut p, Instruction::Rd8)?,
            "RD16" => extend_2reg(&mut bytecode, &mut p, Instruction::Rd16)?,
            "RD32" => extend_2reg(&mut bytecode, &mut p, Instruction::Rd32)?,

            // write (TODO: rename to store?)
            "WR8" => extend_2reg(&mut bytecode, &mut p, Instruction::Wr8)?,
            "WR16" => extend_2reg(&mut bytecode, &mut p, Instruction::Wr16)?,
            "WR32" => extend_2reg(&mut bytecode, &mut p, Instruction::Wr32)?,

            // arithmetic
            "ADD" => extend_2reg(&mut bytecode, &mut p, Instruction::Add)?,
            "SUB" => extend_2reg(&mut bytecode, &mut p, Instruction::Sub)?,
            "MUL" => extend_2reg(&mut bytecode, &mut p, Instruction::Mul)?,
            "DIV" => extend_2reg(&mut bytecode, &mut p, Instruction::Div)?,
            "SHL" => extend_2reg(&mut bytecode, &mut p, Instruction::Shl)?,
            "SHR" => extend_2reg(&mut bytecode, &mut p, Instruction::Shr)?,

            // comparisons
            "EQ" => extend_2reg(&mut bytecode, &mut p, Instruction::Eq)?,
            "NEQ" => extend_2reg(&mut bytecode, &mut p, Instruction::Neq)?,
            "LT" => extend_2reg(&mut bytecode, &mut p, Instruction::Lt)?,
            "LEQ" => extend_2reg(&mut bytecode, &mut p, Instruction::Leq)?,
            "GT" => extend_2reg(&mut bytecode, &mut p, Instruction::Gt)?,
            "GEQ" => extend_2reg(&mut bytecode, &mut p, Instruction::Geq)?,

            "NOT" => extend(&mut bytecode, Instruction::Not, &[]),

            // debug
            "PRINT" => {
                let reg = p.read_reg().ok_or_else(|| format!("not a register"))?;
                bytecode.extend_from_slice(&[Instruction::Print as u8, reg]);
            }

            _ => return Err(format!("invalid word {:?}", w)),
        }
    }

    Ok(bytecode)
}

struct Parser<'a> {
    src: &'a str,
    idx: usize,
}

impl<'a> Parser<'a> {
    pub fn new(src: &'a str) -> Self {
        Self { src, idx: 0 }
    }

    fn skip_whitespace(&mut self) {
        let string = &self.src[self.idx..];

        let mut last_end = 0usize;
        for (start, end, ch) in Utf8CharIterator::new(string) {
            last_end = end;
            if !is_whitespace(ch) {
                self.idx += start;
                return;
            }
        }
        self.idx += last_end;
    }

    pub fn read_word(&mut self) -> Option<&'a str> {
        self.skip_whitespace();

        let string = &self.src[self.idx..];

        for (start, _, ch) in Utf8CharIterator::new(string) {
            if is_whitespace(ch) {
                self.idx += start;
                return Some(&string[..start]);
            }
        }

        None
    }

    pub fn read_reg(&mut self) -> Option<u8> {
        self.read_word()
            .and_then(|w| {
                if w.starts_with("r") {
                    Some(&w[1..])
                } else {
                    None
                }
            })
            .and_then(|w| w.parse::<u8>().ok())
            .and_then(|n| if n & 0b111 == n { Some(n) } else { None })
    }

    pub fn read_u32(&mut self) -> Option<u32> {
        self.read_word().and_then(|w| {
            if w.starts_with("#") {
                u32::from_str_radix(&w[1..], 16).ok()
            } else {
                u32::from_str_radix(w, 10).ok()
            }
        })
    }
}

fn is_whitespace(ch: char) -> bool {
    match ch {
        ' ' => true,
        '\t' => true,
        '\n' => true,
        _ => false,
    }
}
