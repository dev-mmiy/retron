//! 入力処理モジュール

use core::sync::atomic::{AtomicU32, Ordering};

/// 入力イベント
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum InputEvent {
    KeyPress { key: KeyCode, modifiers: KeyModifiers },
    KeyRelease { key: KeyCode, modifiers: KeyModifiers },
    MouseMove { x: i32, y: i32 },
    MousePress { button: MouseButton, x: i32, y: i32 },
    MouseRelease { button: MouseButton, x: i32, y: i32 },
    TouchStart { x: i32, y: i32, id: u32 },
    TouchMove { x: i32, y: i32, id: u32 },
    TouchEnd { x: i32, y: i32, id: u32 },
}

/// キーコード
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum KeyCode {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space, Enter, Escape, Backspace, Tab,
    ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Unknown,
}

/// キー修飾子
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct KeyModifiers {
    pub ctrl: bool,
    pub alt: bool,
    pub shift: bool,
    pub meta: bool,
}

impl KeyModifiers {
    pub fn new() -> Self {
        Self {
            ctrl: false,
            alt: false,
            shift: false,
            meta: false,
        }
    }
}

/// マウスボタン
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum MouseButton {
    Left,
    Right,
    Middle,
    Unknown,
}

/// 入力システム
pub struct InputSystem {
    pub events: Vec<InputEvent>,
    pub mouse_x: i32,
    pub mouse_y: i32,
    pub mouse_buttons: u8,
    pub key_states: [bool; 256],
}

impl InputSystem {
    /// 新しい入力システムを作成
    pub fn new() -> Self {
        Self {
            events: Vec::new(),
            mouse_x: 0,
            mouse_y: 0,
            mouse_buttons: 0,
            key_states: [false; 256],
        }
    }

    /// イベントを追加
    pub fn add_event(&mut self, event: InputEvent) {
        self.events.push(event);
    }

    /// イベントを取得
    pub fn get_event(&mut self) -> Option<InputEvent> {
        self.events.pop()
    }

    /// キーが押されているかチェック
    pub fn is_key_pressed(&self, key: KeyCode) -> bool {
        let index = key as usize;
        if index < self.key_states.len() {
            self.key_states[index]
        } else {
            false
        }
    }

    /// マウスボタンが押されているかチェック
    pub fn is_mouse_button_pressed(&self, button: MouseButton) -> bool {
        let mask = match button {
            MouseButton::Left => 0x01,
            MouseButton::Right => 0x02,
            MouseButton::Middle => 0x04,
            MouseButton::Unknown => 0x00,
        };
        (self.mouse_buttons & mask) != 0
    }

    /// マウス位置を取得
    pub fn get_mouse_position(&self) -> (i32, i32) {
        (self.mouse_x, self.mouse_y)
    }

    /// イベントをクリア
    pub fn clear_events(&mut self) {
        self.events.clear();
    }
}

/// グローバル入力システム
static mut INPUT_SYSTEM: Option<InputSystem> = None;

/// 入力システムの初期化
pub fn init() {
    unsafe {
        INPUT_SYSTEM = Some(InputSystem::new());
    }
}

/// 入力システムを取得
pub fn get_input_system() -> Option<&'static mut InputSystem> {
    unsafe {
        INPUT_SYSTEM.as_mut()
    }
}

/// 入力処理
pub fn process_input() {
    unsafe {
        if let Some(ref mut system) = INPUT_SYSTEM {
            // 入力処理
            // TODO: 実際の入力処理を実装
        }
    }
}

/// キーイベントを処理
pub fn handle_key_event(key: KeyCode, pressed: bool, modifiers: KeyModifiers) {
    unsafe {
        if let Some(ref mut system) = INPUT_SYSTEM {
            let event = if pressed {
                InputEvent::KeyPress { key, modifiers }
            } else {
                InputEvent::KeyRelease { key, modifiers }
            };
            system.add_event(event);
            
            // キー状態を更新
            let index = key as usize;
            if index < system.key_states.len() {
                system.key_states[index] = pressed;
            }
        }
    }
}

/// マウスイベントを処理
pub fn handle_mouse_event(button: MouseButton, pressed: bool, x: i32, y: i32) {
    unsafe {
        if let Some(ref mut system) = INPUT_SYSTEM {
            let event = if pressed {
                InputEvent::MousePress { button, x, y }
            } else {
                InputEvent::MouseRelease { button, x, y }
            };
            system.add_event(event);
            
            // マウス位置を更新
            system.mouse_x = x;
            system.mouse_y = y;
            
            // マウスボタン状態を更新
            let mask = match button {
                MouseButton::Left => 0x01,
                MouseButton::Right => 0x02,
                MouseButton::Middle => 0x04,
                MouseButton::Unknown => 0x00,
            };
            if pressed {
                system.mouse_buttons |= mask;
            } else {
                system.mouse_buttons &= !mask;
            }
        }
    }
}

/// タッチイベントを処理
pub fn handle_touch_event(event_type: TouchEventType, x: i32, y: i32, id: u32) {
    unsafe {
        if let Some(ref mut system) = INPUT_SYSTEM {
            let event = match event_type {
                TouchEventType::Start => InputEvent::TouchStart { x, y, id },
                TouchEventType::Move => InputEvent::TouchMove { x, y, id },
                TouchEventType::End => InputEvent::TouchEnd { x, y, id },
            };
            system.add_event(event);
        }
    }
}

/// タッチイベントタイプ
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum TouchEventType {
    Start,
    Move,
    End,
}


