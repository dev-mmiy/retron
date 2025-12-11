//! シンプルなカーネル（デバッグ用）

use core::arch::asm;

/// シンプルなHello World表示
pub fn simple_hello() {
    println("Hello, Retron OS!");
}

/// グローバルカーソル位置
static mut CURSOR_POS: usize = 0;

/// 文字列をVGAバッファに出力（シンプル版）
pub fn print(text: &str) {
    let vga_buffer = 0xB8000 as *mut u16;
    
    unsafe {
        for &byte in text.as_bytes().iter() {
            if CURSOR_POS < 2000 { // VGA画面の最大文字数 (25行 * 80列)
                if byte == b'\n' {
                    // 改行処理：次の行の開始位置に移動
                    CURSOR_POS = ((CURSOR_POS / 80) + 1) * 80;
                } else {
                    // 文字を出力（白文字、黒背景）
                    *vga_buffer.add(CURSOR_POS) = 0x0F00 | (byte as u16);
                    CURSOR_POS += 1;
                }
            }
        }
    }
}

/// 文字列をVGAバッファに出力して改行
pub fn println(text: &str) {
    print(text);
    print("\n");
}

/// シリアルポート経由での出力（VGAバッファに出力）
pub fn serial_print(text: &str) {
    // WSL環境ではシリアルポートが制限されるため、VGAバッファに出力
    print(text);
}

/// シリアルポートの初期化（簡易版）
pub fn init_serial() {
    // WSL環境ではシリアルポートが制限されるため、何もしない
}

/// シリアルポート経由での出力（改行付き）
pub fn serial_println(text: &str) {
    serial_print(text);
    serial_print("\n");
}

/// システム停止
pub fn halt() {
    loop {
        unsafe {
            asm!("hlt");
        }
    }
}