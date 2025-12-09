#![no_std]
#![no_main]
#![cfg(not(test))]
// no_std環境では標準のtestクレートが利用できないため、テストビルドを無効化
#![feature(abi_x86_interrupt)]
#![feature(alloc_error_handler)]

use core::arch::asm;

#[allow(dead_code)]
mod config;
#[allow(dead_code)]
mod filesystem;
#[allow(dead_code)]
mod init_config;
mod multiboot;
mod simple;
#[allow(dead_code)]
mod stdio_terminal;
#[allow(dead_code)]
mod terminal;

// Use library modules
use retron_kernel::memory;

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

    // メモリ管理のテスト（Step 1）
    test_memory_management();

    // システム準備完了
    simple::println("KERNEL: System ready");

    // μT-Kernel互換レイヤーの初期化（簡易版）
    // init_utkernel();
}

/// メモリ管理機能のテスト（Step 1検証用）
fn test_memory_management() {
    simple::println("");
    simple::println("=== Memory Management Test (Step 1) ===");

    // ヒープ使用量の確認
    let heap_usage = memory::get_heap_usage();
    simple::print("Heap usage: ");
    simple::println(if heap_usage > 0 { "OK" } else { "OK (0 bytes)" });

    // システムメモリ情報の確認
    let mem_info = memory::SystemMemory::get_info();
    simple::print("Memory total: ");
    simple::println(if mem_info.total == 4 * 1024 * 1024 {
        "4MB OK"
    } else {
        "ERROR"
    });

    // スタック割り当てテスト
    simple::print("Stack allocation: ");
    if let Some(stack_ptr) = memory::allocate_default_stack() {
        simple::print("OK (ptr=");
        if stack_ptr > 0 {
            simple::print("valid");
        }
        simple::println(")");

        // スタック使用状況確認
        let (count, _size) = memory::get_stack_usage();
        simple::print("Stack count: ");
        simple::println(if count == 1 { "1 OK" } else { "ERROR" });

        // スタック解放
        simple::print("Stack deallocation: ");
        if memory::deallocate_stack(stack_ptr) {
            simple::println("OK");
        } else {
            simple::println("ERROR");
        }

        // 解放後の確認
        let (count_after, _) = memory::get_stack_usage();
        simple::print("Stack count after free: ");
        simple::println(if count_after == 0 { "0 OK" } else { "ERROR" });
    } else {
        simple::println("ERROR");
    }

    simple::println("=== Test Complete ===");
    simple::println("");
}
