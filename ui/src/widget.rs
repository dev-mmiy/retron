//! ウィジェットシステム

use crate::graphics::Rgb565;
use crate::input::{InputEvent, KeyCode, MouseButton};

/// ウィジェットID
pub type WidgetId = usize;

/// ウィジェット状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum WidgetState {
    Normal,
    Hovered,
    Pressed,
    Disabled,
}

/// ウィジェット位置とサイズ
#[derive(Debug, Clone, Copy)]
pub struct WidgetRect {
    pub x: i32,
    pub y: i32,
    pub width: u32,
    pub height: u32,
}

/// ウィジェット基底トレイト
pub trait Widget {
    /// ウィジェットIDを取得
    fn get_id(&self) -> WidgetId;
    
    /// ウィジェットの位置とサイズを取得
    fn get_rect(&self) -> WidgetRect;
    
    /// ウィジェットの状態を取得
    fn get_state(&self) -> WidgetState;
    
    /// ウィジェットを描画
    fn draw(&self, graphics: &mut crate::graphics::GraphicsSystem);
    
    /// イベントを処理
    fn handle_event(&mut self, event: InputEvent) -> bool;
    
    /// ウィジェットを更新
    fn update(&mut self);
}

/// ボタンウィジェット
pub struct Button {
    pub id: WidgetId,
    pub rect: WidgetRect,
    pub state: WidgetState,
    pub text: String,
    pub background_color: Rgb565,
    pub text_color: Rgb565,
    pub on_click: Option<fn()>,
}

impl Button {
    /// 新しいボタンを作成
    pub fn new(id: WidgetId, x: i32, y: i32, width: u32, height: u32, text: String) -> Self {
        Self {
            id,
            rect: WidgetRect { x, y, width, height },
            state: WidgetState::Normal,
            text,
            background_color: Rgb565::new(200, 200, 200),
            text_color: Rgb565::new(0, 0, 0),
            on_click: None,
        }
    }

    /// クリックハンドラーを設定
    pub fn set_on_click(&mut self, handler: fn()) {
        self.on_click = Some(handler);
    }
}

impl Widget for Button {
    fn get_id(&self) -> WidgetId {
        self.id
    }

    fn get_rect(&self) -> WidgetRect {
        self.rect
    }

    fn get_state(&self) -> WidgetState {
        self.state
    }

    fn draw(&self, graphics: &mut crate::graphics::GraphicsSystem) {
        // 背景を描画
        graphics.set_color(self.background_color);
        graphics.draw_rectangle(
            self.rect.x as u32,
            self.rect.y as u32,
            self.rect.width,
            self.rect.height,
        );

        // テキストを描画
        graphics.set_color(self.text_color);
        graphics.draw_text(
            self.rect.x as u32 + 10,
            self.rect.y as u32 + 20,
            &self.text,
        );
    }

    fn handle_event(&mut self, event: InputEvent) -> bool {
        match event {
            InputEvent::MousePress { button: MouseButton::Left, x, y } => {
                if self.is_point_inside(x, y) {
                    self.state = WidgetState::Pressed;
                    if let Some(handler) = self.on_click {
                        handler();
                    }
                    return true;
                }
            },
            InputEvent::MouseRelease { button: MouseButton::Left, x, y } => {
                if self.state == WidgetState::Pressed {
                    self.state = WidgetState::Normal;
                    return true;
                }
            },
            _ => {}
        }
        false
    }

    fn update(&mut self) {
        // ボタンの更新処理
    }
}

impl Button {
    /// ポイントがウィジェット内にあるかチェック
    fn is_point_inside(&self, x: i32, y: i32) -> bool {
        x >= self.rect.x && x < self.rect.x + self.rect.width as i32 &&
        y >= self.rect.y && y < self.rect.y + self.rect.height as i32
    }
}

/// ラベルウィジェット
pub struct Label {
    pub id: WidgetId,
    pub rect: WidgetRect,
    pub state: WidgetState,
    pub text: String,
    pub text_color: Rgb565,
}

impl Label {
    /// 新しいラベルを作成
    pub fn new(id: WidgetId, x: i32, y: i32, text: String) -> Self {
        Self {
            id,
            rect: WidgetRect { x, y, width: 100, height: 20 },
            state: WidgetState::Normal,
            text,
            text_color: Rgb565::new(0, 0, 0),
        }
    }
}

impl Widget for Label {
    fn get_id(&self) -> WidgetId {
        self.id
    }

    fn get_rect(&self) -> WidgetRect {
        self.rect
    }

    fn get_state(&self) -> WidgetState {
        self.state
    }

    fn draw(&self, graphics: &mut crate::graphics::GraphicsSystem) {
        graphics.set_color(self.text_color);
        graphics.draw_text(
            self.rect.x as u32,
            self.rect.y as u32,
            &self.text,
        );
    }

    fn handle_event(&mut self, _event: InputEvent) -> bool {
        false
    }

    fn update(&mut self) {
        // ラベルの更新処理
    }
}

/// ウィジェット管理システム
pub struct WidgetManager {
    widgets: Vec<Box<dyn Widget>>,
    next_widget_id: AtomicUsize,
}

impl WidgetManager {
    /// 新しいウィジェットマネージャーを作成
    pub fn new() -> Self {
        Self {
            widgets: Vec::new(),
            next_widget_id: AtomicUsize::new(1),
        }
    }

    /// ウィジェットを追加
    pub fn add_widget(&mut self, widget: Box<dyn Widget>) {
        self.widgets.push(widget);
    }

    /// ウィジェットを取得
    pub fn get_widget(&mut self, id: WidgetId) -> Option<&mut dyn Widget> {
        self.widgets.iter_mut().find(|w| w.get_id() == id).map(|w| w.as_mut())
    }

    /// ウィジェットを削除
    pub fn remove_widget(&mut self, id: WidgetId) -> bool {
        if let Some(pos) = self.widgets.iter().position(|w| w.get_id() == id) {
            self.widgets.remove(pos);
            true
        } else {
            false
        }
    }

    /// 全ウィジェットを描画
    pub fn draw_all(&mut self, graphics: &mut crate::graphics::GraphicsSystem) {
        for widget in &self.widgets {
            widget.draw(graphics);
        }
    }

    /// イベントを処理
    pub fn handle_event(&mut self, event: InputEvent) -> bool {
        for widget in &mut self.widgets {
            if widget.handle_event(event) {
                return true;
            }
        }
        false
    }

    /// 全ウィジェットを更新
    pub fn update_all(&mut self) {
        for widget in &mut self.widgets {
            widget.update();
        }
    }
}

/// グローバルウィジェットマネージャー
static mut WIDGET_MANAGER: Option<WidgetManager> = None;

/// ウィジェットシステムの初期化
pub fn init() {
    unsafe {
        WIDGET_MANAGER = Some(WidgetManager::new());
    }
}

/// ウィジェットマネージャーを取得
pub fn get_widget_manager() -> Option<&'static mut WidgetManager> {
    unsafe {
        WIDGET_MANAGER.as_mut()
    }
}

/// ボタンを作成
pub fn create_button(x: i32, y: i32, width: u32, height: u32, text: String) -> Option<WidgetId> {
    unsafe {
        if let Some(ref mut manager) = WIDGET_MANAGER {
            let id = manager.next_widget_id.fetch_add(1, Ordering::SeqCst);
            let button = Button::new(id, x, y, width, height, text);
            manager.add_widget(Box::new(button));
            Some(id)
        } else {
            None
        }
    }
}

/// ラベルを作成
pub fn create_label(x: i32, y: i32, text: String) -> Option<WidgetId> {
    unsafe {
        if let Some(ref mut manager) = WIDGET_MANAGER {
            let id = manager.next_widget_id.fetch_add(1, Ordering::SeqCst);
            let label = Label::new(id, x, y, text);
            manager.add_widget(Box::new(label));
            Some(id)
        } else {
            None
        }
    }
}


