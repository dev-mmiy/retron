//! Boot stack for PVH entry point
//!
//! This module defines the boot stack as a Rust static array,
//! which ensures it's properly allocated in the binary's .bss section.

use core::mem::MaybeUninit;

/// Boot stack size (16KB)
const BOOT_STACK_SIZE: usize = 16384;

/// Boot stack storage
/// Placed in .bss section and zero-initialized by the loader
#[no_mangle]
#[used]
#[link_section = ".bss.boot_stack"]
static mut BOOT_STACK: [u8; BOOT_STACK_SIZE] = [0; BOOT_STACK_SIZE];

/// Get the address of the top of the boot stack
/// The stack grows downward, so this returns the highest address
#[no_mangle]
pub unsafe extern "C" fn get_boot_stack_top() -> *mut u8 {
    BOOT_STACK.as_mut_ptr().add(BOOT_STACK_SIZE) as *mut u8
}
