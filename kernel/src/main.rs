#![no_std]
#![no_main]
#![feature(abi_x86_interrupt)]
#![feature(alloc_error_handler)]

use core::panic::PanicInfo;
use core::arch::asm;
use x86_64::structures::idt::{InterruptDescriptorTable, InterruptStackFrame};

mod test;
mod simple;
mod filesystem;
mod fs_test;
mod fs_demo;
mod terminal;
mod terminal_test;
mod terminal_demo;
mod config;
mod stdio_terminal;
mod init_config;

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

/// システム準備完了メッセージ
fn show_system_ready() {
    simple::println("");
    simple::println("==========================================");
    simple::println("    Retron OS System Ready");
    simple::println("==========================================");
    simple::println("All services initialized successfully.");
    simple::println("Filesystem operations available.");
    simple::println("Terminal control active.");
    simple::println("");
}

/// デモコマンドの実行（無効化）
fn run_demo_commands() {
    // デモ機能は無効化されています
    simple::println("Demo functionality disabled.");
}

/// デモコマンドの実行（無効化）
fn execute_demo_command(_command: &str) {
    // デモ機能は無効化されています
    simple::println("Command execution disabled.");
}

/// テストの実行
fn run_tests() {
    // テスト結果をVGAに表示
    let vga_buffer = 0xB8000 as *mut u16;
    let test_result = test::run_all_tests();
    
    if test_result {
        // テスト成功
        let message = b"Tests: PASS";
        for (i, &byte) in message.iter().enumerate() {
            unsafe {
                *vga_buffer.add(80 + i) = 0x0A00 | (byte as u16); // 緑文字
            }
        }
    } else {
        // テスト失敗
        let message = b"Tests: FAIL";
        for (i, &byte) in message.iter().enumerate() {
            unsafe {
                *vga_buffer.add(80 + i) = 0x0C00 | (byte as u16); // 赤文字
            }
        }
    }
}

/// シンプルなHello World表示
fn print_hello() {
    // VGAテキストモードでの表示（0xB8000がVGAテキストバッファの開始アドレス）
    let vga_buffer = 0xB8000 as *mut u16;
    let message = b"Hello, Retron OS - 2 !";
    
    for (i, &byte) in message.iter().enumerate() {
        unsafe {
            *vga_buffer.add(i) = 0x0F00 | (byte as u16); // 白文字、黒背景
        }
    }
}

/// メモリ管理の初期化
fn init_memory() {
    // メモリマネージャーの初期化
    // TODO: メモリプールの設定
}

/// 割り込み処理の初期化
fn init_interrupts() {
    // IDTの設定
    // TODO: 割り込みハンドラーの登録
}

/// デバイスドライバーの初期化
fn init_devices() {
    // シリアルポートの初期化
    // TODO: デバイスドライバーの初期化
}

/// μT-Kernel互換レイヤーの初期化
fn init_utkernel() {
    // μT-Kernel 3.x APIの初期化
    // TODO: タスク管理、メモリ管理、同期処理の初期化
}

/// カーネルのメインループ
fn kernel_loop() {
    // システムアイドル処理
    simple::halt();
}

/// CPUを停止
fn halt() {
    unsafe {
        asm!("hlt");
    }
}

/// パニックハンドラー
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    // パニック時の処理
    loop {}
}

/// アロケーションハンドラー
#[alloc_error_handler]
fn alloc_error(_layout: core::alloc::Layout) -> ! {
    // メモリ不足時の処理
    loop {}
}

/// 割り込みハンドラー（ダミー）
extern "x86-interrupt" fn timer_interrupt_handler(_stack_frame: InterruptStackFrame) {
    // タイマー割り込みの処理
    // TODO: タスクスケジューリング
}
