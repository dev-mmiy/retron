//! ウィンドウ管理モジュール

use core::sync::atomic::{AtomicUsize, Ordering};

/// ウィンドウID
pub type WindowId = usize;

/// ウィンドウ状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum WindowState {
    Normal,
    Minimized,
    Maximized,
    Hidden,
}

/// ウィンドウ構造体
pub struct Window {
    pub id: WindowId,
    pub title: String,
    pub x: i32,
    pub y: i32,
    pub width: u32,
    pub height: u32,
    pub state: WindowState,
    pub visible: bool,
    pub focused: bool,
}

impl Window {
    /// 新しいウィンドウを作成
    pub fn new(id: WindowId, title: String, x: i32, y: i32, width: u32, height: u32) -> Self {
        Self {
            id,
            title,
            x,
            y,
            width,
            height,
            state: WindowState::Normal,
            visible: true,
            focused: false,
        }
    }

    /// ウィンドウを移動
    pub fn move_to(&mut self, x: i32, y: i32) {
        self.x = x;
        self.y = y;
    }

    /// ウィンドウサイズを変更
    pub fn resize(&mut self, width: u32, height: u32) {
        self.width = width;
        self.height = height;
    }

    /// ウィンドウを表示
    pub fn show(&mut self) {
        self.visible = true;
    }

    /// ウィンドウを非表示
    pub fn hide(&mut self) {
        self.visible = false;
    }

    /// ウィンドウを最小化
    pub fn minimize(&mut self) {
        self.state = WindowState::Minimized;
    }

    /// ウィンドウを最大化
    pub fn maximize(&mut self) {
        self.state = WindowState::Maximized;
    }

    /// ウィンドウを元に戻す
    pub fn restore(&mut self) {
        self.state = WindowState::Normal;
    }

    /// ウィンドウにフォーカス
    pub fn focus(&mut self) {
        self.focused = true;
    }

    /// ウィンドウからフォーカスを外す
    pub fn unfocus(&mut self) {
        self.focused = false;
    }
}

/// ウィンドウ管理システム
pub struct WindowManager {
    windows: Vec<Window>,
    next_window_id: AtomicUsize,
    focused_window: Option<WindowId>,
}

impl WindowManager {
    /// 新しいウィンドウマネージャーを作成
    pub fn new() -> Self {
        Self {
            windows: Vec::new(),
            next_window_id: AtomicUsize::new(1),
            focused_window: None,
        }
    }

    /// ウィンドウを作成
    pub fn create_window(&mut self, title: String, x: i32, y: i32, width: u32, height: u32) -> WindowId {
        let id = self.next_window_id.fetch_add(1, Ordering::SeqCst);
        let window = Window::new(id, title, x, y, width, height);
        self.windows.push(window);
        id
    }

    /// ウィンドウを取得
    pub fn get_window(&mut self, id: WindowId) -> Option<&mut Window> {
        self.windows.iter_mut().find(|w| w.id == id)
    }

    /// ウィンドウを削除
    pub fn destroy_window(&mut self, id: WindowId) -> bool {
        if let Some(pos) = self.windows.iter().position(|w| w.id == id) {
            self.windows.remove(pos);
            if self.focused_window == Some(id) {
                self.focused_window = None;
            }
            true
        } else {
            false
        }
    }

    /// ウィンドウ一覧を取得
    pub fn get_windows(&self) -> &Vec<Window> {
        &self.windows
    }

    /// フォーカスされたウィンドウを取得
    pub fn get_focused_window(&self) -> Option<WindowId> {
        self.focused_window
    }

    /// ウィンドウにフォーカス
    pub fn focus_window(&mut self, id: WindowId) -> bool {
        if let Some(window) = self.get_window(id) {
            // 現在のフォーカスを外す
            if let Some(current_id) = self.focused_window {
                if let Some(current_window) = self.get_window(current_id) {
                    current_window.unfocus();
                }
            }
            
            // 新しいウィンドウにフォーカス
            window.focus();
            self.focused_window = Some(id);
            true
        } else {
            false
        }
    }

    /// ウィンドウを最前面に移動
    pub fn bring_to_front(&mut self, id: WindowId) -> bool {
        if let Some(pos) = self.windows.iter().position(|w| w.id == id) {
            let window = self.windows.remove(pos);
            self.windows.push(window);
            true
        } else {
            false
        }
    }

    /// ウィンドウを描画
    pub fn render_windows(&mut self) {
        for window in &self.windows {
            if window.visible {
                // ウィンドウの描画処理
                // TODO: 実際の描画処理を実装
            }
        }
    }
}

/// グローバルウィンドウマネージャー
static mut WINDOW_MANAGER: Option<WindowManager> = None;

/// ウィンドウシステムの初期化
pub fn init() {
    unsafe {
        WINDOW_MANAGER = Some(WindowManager::new());
    }
}

/// ウィンドウマネージャーを取得
pub fn get_window_manager() -> Option<&'static mut WindowManager> {
    unsafe {
        WINDOW_MANAGER.as_mut()
    }
}

/// ウィンドウを作成
pub fn create_window(title: String, x: i32, y: i32, width: u32, height: u32) -> Option<WindowId> {
    unsafe {
        if let Some(ref mut manager) = WINDOW_MANAGER {
            Some(manager.create_window(title, x, y, width, height))
        } else {
            None
        }
    }
}

/// ウィンドウを取得
pub fn get_window(id: WindowId) -> Option<&'static mut Window> {
    unsafe {
        if let Some(ref mut manager) = WINDOW_MANAGER {
            manager.get_window(id)
        } else {
            None
        }
    }
}

/// ウィンドウを削除
pub fn destroy_window(id: WindowId) -> bool {
    unsafe {
        if let Some(ref mut manager) = WINDOW_MANAGER {
            manager.destroy_window(id)
        } else {
            false
        }
    }
}

/// ウィンドウにフォーカス
pub fn focus_window(id: WindowId) -> bool {
    unsafe {
        if let Some(ref mut manager) = WINDOW_MANAGER {
            manager.focus_window(id)
        } else {
            false
        }
    }
}


