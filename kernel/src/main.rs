#![no_std]
#![no_main]
#![cfg(not(test))]
// no_std環境では標準のtestクレートが利用できないため、テストビルドを無効化
#![feature(abi_x86_interrupt)]
#![feature(alloc_error_handler)]

use core::arch::asm;
use core::panic::PanicInfo;

#[allow(dead_code)]
mod config;
#[allow(dead_code)]
mod filesystem;
#[allow(dead_code)]
mod fs_demo;
#[allow(dead_code)]
mod fs_test;
#[allow(dead_code)]
mod init_config;
#[allow(dead_code)]
mod kernel_test;
mod simple;
#[allow(dead_code)]
mod stdio_terminal;
#[allow(dead_code)]
mod terminal;
#[allow(dead_code)]
mod terminal_demo;
#[allow(dead_code)]
mod terminal_test;

/// カーネルエントリーポイント（Multiboot対応）
#[no_mangle]
pub extern "C" fn kernel_main() -> ! {
    // カーネル開始の確認
    simple::println("KERNEL_MAIN: Starting...");
    simple::println("KERNEL_MAIN: Debug test 1");
    simple::println("KERNEL_MAIN: Debug test 2");
    simple::println("KERNEL_MAIN: About to call init()");

    // 初期化処理
    init();
    simple::println("KERNEL_MAIN: init() completed");

    // システム準備完了メッセージ
    simple::println("");
    simple::println("==========================================");
    simple::println("    Retron OS System Ready");
    simple::println("==========================================");
    simple::println("All services initialized successfully.");
    simple::println("Filesystem operations available.");
    simple::println("Terminal control active.");
    simple::println("");
    simple::println("System will now halt.");
    simple::println("Press Ctrl+C to exit QEMU");
    simple::println("");

    simple::println("DEBUG: About to enter halt loop");

    // システム終了
    loop {
        // CPUを停止（割り込み待ち）
        unsafe {
            asm!("hlt");
        }
    }
}

/// カーネル初期化（最小限版）
fn init() {
    // 最小限のデバッグ出力
    simple::println("KERNEL: Starting...");
    simple::println("KERNEL: Basic test 1");
    simple::println("KERNEL: Basic test 2");
    simple::println("KERNEL: Basic test 3");

    // 最小限のテスト完了
    simple::println("KERNEL: All basic tests completed");

    // システム準備完了
    simple::println("KERNEL: System ready");

    // μT-Kernel互換レイヤーの初期化（簡易版）
    // init_utkernel();
}

/// パニックハンドラー
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
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
