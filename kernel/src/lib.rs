//! Retron OS Kernel
//!
//! μT-Kernel 3.xベースのモダンカーネル実装

#![no_std]
#![cfg(not(test))]
// no_std環境では標準のtestクレートが利用できないため、テストビルドを無効化
#![feature(abi_x86_interrupt)]
#![feature(alloc_error_handler)]

pub mod config;
pub mod device;
pub mod filesystem;
pub mod fs_demo;
pub mod fs_test;
pub mod init_config;
pub mod interrupt;
pub mod kernel_test;
pub mod memory;
pub mod prelude;
pub mod simple;
pub mod stdio_terminal;
pub mod task;
pub mod terminal;
pub mod terminal_demo;
pub mod terminal_test;
pub mod utkernel;

/// カーネルバージョン情報
pub const KERNEL_VERSION: &str = "0.1.0";
pub const UTKERNEL_VERSION: &str = "3.0";

/// パニックハンドラー
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    // パニック時の処理 - CPUをhltで停止
    loop {
        unsafe {
            core::arch::asm!("hlt");
        }
    }
}

/// アロケーションハンドラー
#[alloc_error_handler]
fn alloc_error(_layout: core::alloc::Layout) -> ! {
    // メモリ不足時の処理 - CPUをhltで停止
    loop {
        unsafe {
            core::arch::asm!("hlt");
        }
    }
}

/// カーネル初期化
pub fn kernel_init() {
    // メモリ管理の初期化
    memory::init();

    // タスク管理の初期化
    task::init();

    // デバイス管理の初期化
    device::init();

    // 割り込み処理の初期化
    interrupt::init();

    // μT-Kernel互換レイヤーの初期化
    utkernel::init();
}

/// カーネルメインループ
pub fn kernel_main_loop() {
    loop {
        // タスクスケジューリング
        task::scheduler();

        // デバイス処理
        device::process();

        // システムアイドル処理
        x86_64::instructions::hlt();
    }
}
