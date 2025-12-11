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
}
