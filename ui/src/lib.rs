//! Retron UI Framework
//! 
//! モダンなUIフレームワーク

pub mod graphics;
pub mod input;
pub mod window;
pub mod widget;
pub mod theme;
pub mod event;

/// UIフレームワークの初期化
pub fn init() {
    // グラフィックスシステムの初期化
    graphics::init();
    
    // 入力システムの初期化
    input::init();
    
    // ウィンドウシステムの初期化
    window::init();
    
    // テーマシステムの初期化
    theme::init();
}

/// UIフレームワークのメインループ
pub fn main_loop() {
    loop {
        // イベント処理
        event::process_events();
        
        // 描画処理
        graphics::render();
        
        // 入力処理
        input::process_input();
    }
}


