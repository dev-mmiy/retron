//! イベント処理システム

use crate::input::InputEvent;

/// アプリケーションイベント
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum AppEvent {
    Quit,
    Resize { width: u32, height: u32 },
    Focus,
    Unfocus,
}

/// イベントハンドラー
pub type EventHandler = fn(InputEvent) -> bool;

/// イベント管理システム
pub struct EventManager {
    pub input_handlers: Vec<EventHandler>,
    pub app_events: Vec<AppEvent>,
}

impl EventManager {
    /// 新しいイベントマネージャーを作成
    pub fn new() -> Self {
        Self {
            input_handlers: Vec::new(),
            app_events: Vec::new(),
        }
    }

    /// 入力イベントハンドラーを登録
    pub fn register_input_handler(&mut self, handler: EventHandler) {
        self.input_handlers.push(handler);
    }

    /// アプリケーションイベントを追加
    pub fn add_app_event(&mut self, event: AppEvent) {
        self.app_events.push(event);
    }

    /// アプリケーションイベントを取得
    pub fn get_app_event(&mut self) -> Option<AppEvent> {
        self.app_events.pop()
    }

    /// イベントを処理
    pub fn process_events(&mut self) {
        // 入力イベントの処理
        // TODO: 実際の入力イベント処理を実装
        
        // アプリケーションイベントの処理
        while let Some(event) = self.get_app_event() {
            match event {
                AppEvent::Quit => {
                    // アプリケーション終了処理
                },
                AppEvent::Resize { width, height } => {
                    // リサイズ処理
                },
                AppEvent::Focus => {
                    // フォーカス処理
                },
                AppEvent::Unfocus => {
                    // フォーカス解除処理
                },
            }
        }
    }
}

/// グローバルイベントマネージャー
static mut EVENT_MANAGER: Option<EventManager> = None;

/// イベントシステムの初期化
pub fn init() {
    unsafe {
        EVENT_MANAGER = Some(EventManager::new());
    }
}

/// イベントマネージャーを取得
pub fn get_event_manager() -> Option<&'static mut EventManager> {
    unsafe {
        EVENT_MANAGER.as_mut()
    }
}

/// イベントを処理
pub fn process_events() {
    unsafe {
        if let Some(ref mut manager) = EVENT_MANAGER {
            manager.process_events();
        }
    }
}

/// 入力イベントハンドラーを登録
pub fn register_input_handler(handler: EventHandler) {
    unsafe {
        if let Some(ref mut manager) = EVENT_MANAGER {
            manager.register_input_handler(handler);
        }
    }
}

/// アプリケーションイベントを追加
pub fn add_app_event(event: AppEvent) {
    unsafe {
        if let Some(ref mut manager) = EVENT_MANAGER {
            manager.add_app_event(event);
        }
    }
}


