#![allow(dead_code)]

use std::process::ExitCode;

mod asm;
mod vm;
mod utf8;
mod byte;

use crate::vm::Vm;

fn main() -> ExitCode {
    match run() {
        Ok(()) => ExitCode::from(0),
        Err(e) => {
            eprintln!("error: {}", e);
            ExitCode::from(1)
        }
    }
}

fn run() -> Result<(), String> {
    let args = std::env::args().collect::<Vec<String>>();

    if args.len() != 2 {
        return Err("need exactly one argument (asm file name)".into());
    }

    let source_code =
        std::fs::read_to_string(&args[1]).map_err(|e| format!("failed to open file: {e}"))?;

    let compiled =
        asm::compile(&source_code).map_err(|e| format!("failed to compile: {e}"))?;

    let mut vm = Vm::new(0x1000);
    vm.load_rom(&compiled, 0x100);
    vm.run()?;

    Ok(())
}
