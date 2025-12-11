//! テーマシステム

use crate::graphics::Rgb565;

/// テーマカラー
#[derive(Debug, Clone, Copy)]
pub struct ThemeColors {
    pub primary: Rgb565,
    pub secondary: Rgb565,
    pub background: Rgb565,
    pub surface: Rgb565,
    pub text: Rgb565,
    pub text_secondary: Rgb565,
    pub border: Rgb565,
    pub accent: Rgb565,
    pub success: Rgb565,
    pub warning: Rgb565,
    pub error: Rgb565,
}

/// テーマ
pub struct Theme {
    pub name: String,
    pub colors: ThemeColors,
    pub font_size: u32,
    pub border_radius: u32,
    pub padding: u32,
    pub margin: u32,
}

impl Theme {
    /// 新しいテーマを作成
    pub fn new(name: String, colors: ThemeColors) -> Self {
        Self {
            name,
            colors,
            font_size: 12,
            border_radius: 4,
            padding: 8,
            margin: 4,
        }
    }
}

/// テーマ管理システム
pub struct ThemeManager {
    pub current_theme: Theme,
    pub themes: Vec<Theme>,
}

impl ThemeManager {
    /// 新しいテーママネージャーを作成
    pub fn new() -> Self {
        let default_theme = Theme::new(
            "Default".to_string(),
            ThemeColors {
                primary: Rgb565::new(0, 120, 215),
                secondary: Rgb565::new(100, 100, 100),
                background: Rgb565::new(240, 240, 240),
                surface: Rgb565::new(255, 255, 255),
                text: Rgb565::new(0, 0, 0),
                text_secondary: Rgb565::new(100, 100, 100),
                border: Rgb565::new(200, 200, 200),
                accent: Rgb565::new(255, 140, 0),
                success: Rgb565::new(0, 200, 0),
                warning: Rgb565::new(255, 165, 0),
                error: Rgb565::new(255, 0, 0),
            },
        );

        Self {
            current_theme: default_theme,
            themes: vec![default_theme],
        }
    }

    /// テーマを追加
    pub fn add_theme(&mut self, theme: Theme) {
        self.themes.push(theme);
    }

    /// テーマを設定
    pub fn set_theme(&mut self, name: &str) -> bool {
        if let Some(theme) = self.themes.iter().find(|t| t.name == name) {
            self.current_theme = theme.clone();
            true
        } else {
            false
        }
    }

    /// 現在のテーマを取得
    pub fn get_current_theme(&self) -> &Theme {
        &self.current_theme
    }

    /// 利用可能なテーマ一覧を取得
    pub fn get_available_themes(&self) -> Vec<&str> {
        self.themes.iter().map(|t| t.name.as_str()).collect()
    }
}

/// グローバルテーママネージャー
static mut THEME_MANAGER: Option<ThemeManager> = None;

/// テーマシステムの初期化
pub fn init() {
    unsafe {
        THEME_MANAGER = Some(ThemeManager::new());
        
        // 追加テーマを登録
        register_additional_themes();
    }
}

/// 追加テーマを登録
fn register_additional_themes() {
    unsafe {
        if let Some(ref mut manager) = THEME_MANAGER {
            // ダークテーマ
            let dark_theme = Theme::new(
                "Dark".to_string(),
                ThemeColors {
                    primary: Rgb565::new(0, 120, 215),
                    secondary: Rgb565::new(100, 100, 100),
                    background: Rgb565::new(30, 30, 30),
                    surface: Rgb565::new(50, 50, 50),
                    text: Rgb565::new(255, 255, 255),
                    text_secondary: Rgb565::new(150, 150, 150),
                    border: Rgb565::new(80, 80, 80),
                    accent: Rgb565::new(255, 140, 0),
                    success: Rgb565::new(0, 200, 0),
                    warning: Rgb565::new(255, 165, 0),
                    error: Rgb565::new(255, 0, 0),
                },
            );
            manager.add_theme(dark_theme);

            // ハイコントラストテーマ
            let high_contrast_theme = Theme::new(
                "High Contrast".to_string(),
                ThemeColors {
                    primary: Rgb565::new(0, 0, 255),
                    secondary: Rgb565::new(128, 128, 128),
                    background: Rgb565::new(255, 255, 255),
                    surface: Rgb565::new(255, 255, 255),
                    text: Rgb565::new(0, 0, 0),
                    text_secondary: Rgb565::new(0, 0, 0),
                    border: Rgb565::new(0, 0, 0),
                    accent: Rgb565::new(255, 0, 0),
                    success: Rgb565::new(0, 128, 0),
                    warning: Rgb565::new(255, 255, 0),
                    error: Rgb565::new(255, 0, 0),
                },
            );
            manager.add_theme(high_contrast_theme);
        }
    }
}

/// テーママネージャーを取得
pub fn get_theme_manager() -> Option<&'static mut ThemeManager> {
    unsafe {
        THEME_MANAGER.as_mut()
    }
}

/// 現在のテーマを取得
pub fn get_current_theme() -> Option<&'static Theme> {
    unsafe {
        if let Some(ref manager) = THEME_MANAGER {
            Some(&manager.current_theme)
        } else {
            None
        }
    }
}

/// テーマを設定
pub fn set_theme(name: &str) -> bool {
    unsafe {
        if let Some(ref mut manager) = THEME_MANAGER {
            manager.set_theme(name)
        } else {
            false
        }
    }
}

/// 利用可能なテーマ一覧を取得
pub fn get_available_themes() -> Vec<&'static str> {
    unsafe {
        if let Some(ref manager) = THEME_MANAGER {
            manager.get_available_themes()
        } else {
            Vec::new()
        }
    }
}


