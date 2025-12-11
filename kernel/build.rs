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
    cc::Build::new()
        .file(boot_asm)
        .flag("-nostdlib")
        .flag("-ffreestanding")
        .compile("boot");
}
