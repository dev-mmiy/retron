use std::env;
use std::path::PathBuf;

fn main() {
    // Get the directory where Cargo.toml is located
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let linker_script = manifest_dir.join("src").join("linker.ld");

    // Tell cargo to pass the linker script to rustc
    println!("cargo:rustc-link-arg=-T{}", linker_script.display());

    // Re-run if the linker script changes
    println!("cargo:rerun-if-changed=src/linker.ld");

    // Compile assembly boot entry point
    let boot_asm = manifest_dir.join("src").join("boot.s");
    println!("cargo:rerun-if-changed=src/boot.s");

    // Use cc crate to assemble the boot.s file
    // CRITICAL: Do NOT use -m64 or -Wa,--64 as they override .code32 directive!
    // The boot code MUST start in 32-bit mode, then .code64 switches to 64-bit
    // We let the assembler directives (.code32, .code64) control the mode
    cc::Build::new()
        .file(boot_asm)
        .flag("-nostdlib")
        .flag("-ffreestanding")
        // DO NOT ADD -m64 or -Wa,--64 here!
        .flag("-Wa,--divide")  // Allow division in assembly
        .compile("boot");
}
