//! 割り込み処理モジュール
//!
//! 割り込みハンドラーの管理と処理

use crate::prelude::*;
use spin::Mutex;
use x86_64::structures::idt::{InterruptDescriptorTable, InterruptStackFrame};

/// 割り込み番号
pub type InterruptNumber = u8;

/// 割り込みハンドラー関数型
pub type InterruptHandler = extern "x86-interrupt" fn(InterruptStackFrame);

/// 割り込み管理
pub struct InterruptManager {
    handlers: [Option<InterruptHandler>; 256],
    idt: InterruptDescriptorTable,
}

impl Default for InterruptManager {
    fn default() -> Self {
        Self::new()
    }
}

impl InterruptManager {
    /// 新しい割り込みマネージャーを作成
    pub fn new() -> Self {
        Self {
            handlers: [None; 256],
            idt: InterruptDescriptorTable::new(),
        }
    }

    /// 割り込みハンドラーを登録
    pub fn register_handler(
        &mut self,
        interrupt_number: InterruptNumber,
        handler: InterruptHandler,
    ) {
        self.handlers[interrupt_number as usize] = Some(handler);
    }

    /// 割り込みハンドラーを実行
    pub fn handle_interrupt(
        &self,
        interrupt_number: InterruptNumber,
        _stack_frame: InterruptStackFrame,
    ) {
        if let Some(_handler) = self.handlers[interrupt_number as usize] {
            // 割り込みハンドラーの呼び出し（簡易実装）
            // handler(stack_frame);
        }
    }

    /// IDTをロード
    pub fn load_idt(&'static mut self) {
        self.idt.load();
    }
}

/// グローバル割り込みマネージャー
static INTERRUPT_MANAGER: Mutex<Option<InterruptManager>> = Mutex::new(None);

/// 割り込み処理の初期化
pub fn init() {
    *INTERRUPT_MANAGER.lock() = Some(InterruptManager::new());

    // 基本的な割り込みハンドラーを登録
    register_basic_handlers();

    // IDTをロード
    // SAFETY: We need a 'static reference for the IDT to work properly.
    // This is safe because INTERRUPT_MANAGER is static and never moved.
    unsafe {
        let ptr = INTERRUPT_MANAGER.lock().as_mut().unwrap() as *mut InterruptManager;
        (*ptr).load_idt();
    }
}

/// 基本的な割り込みハンドラーの登録
fn register_basic_handlers() {
    if let Some(ref mut manager) = INTERRUPT_MANAGER.lock().as_mut() {
        // タイマー割り込み (IRQ0)
        manager.register_handler(32, timer_interrupt_handler);

        // キーボード割り込み (IRQ1)
        manager.register_handler(33, keyboard_interrupt_handler);

        // シリアル割り込み (IRQ3, IRQ4)
        manager.register_handler(35, serial_interrupt_handler);
        manager.register_handler(36, serial_interrupt_handler);
    }
}

/// 割り込みハンドラーを登録
pub fn register_handler(interrupt_number: InterruptNumber, handler: InterruptHandler) {
    if let Some(ref mut manager) = INTERRUPT_MANAGER.lock().as_mut() {
        manager.register_handler(interrupt_number, handler);
    }
}

/// タイマー割り込みハンドラー
extern "x86-interrupt" fn timer_interrupt_handler(_stack_frame: InterruptStackFrame) {
    // タイマー割り込みの処理
    // TODO: タスクスケジューリングのトリガー
}

/// キーボード割り込みハンドラー
extern "x86-interrupt" fn keyboard_interrupt_handler(_stack_frame: InterruptStackFrame) {
    // キーボード割り込みの処理
    // TODO: キー入力の処理
}

/// シリアル割り込みハンドラー
extern "x86-interrupt" fn serial_interrupt_handler(_stack_frame: InterruptStackFrame) {
    // シリアル割り込みの処理
    // TODO: シリアル通信の処理
}

/// 汎用割り込みハンドラー
#[allow(dead_code)]
extern "x86-interrupt" fn generic_interrupt_handler(_stack_frame: InterruptStackFrame) {
    // 汎用割り込みの処理
    // TODO: 割り込み番号に応じた処理
}
