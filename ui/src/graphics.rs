//! グラフィックス処理モジュール

use embedded_graphics::{
    pixelcolor::Rgb565,
    prelude::*,
    primitives::{Circle, Rectangle, Triangle},
    text::Text,
    mono_font::{MonoTextStyle, ascii::FONT_6X10},
};

/// グラフィックスコンテキスト
pub struct GraphicsContext {
    pub width: u32,
    pub height: u32,
    pub buffer: Vec<u8>,
}

impl GraphicsContext {
    /// 新しいグラフィックスコンテキストを作成
    pub fn new(width: u32, height: u32) -> Self {
        let buffer_size = (width * height * 3) as usize; // RGB24
        Self {
            width,
            height,
            buffer: vec![0; buffer_size],
        }
    }

    /// ピクセルを設定
    pub fn set_pixel(&mut self, x: u32, y: u32, color: Rgb565) {
        if x < self.width && y < self.height {
            let index = ((y * self.width + x) * 3) as usize;
            self.buffer[index] = (color.r() >> 3) as u8;
            self.buffer[index + 1] = (color.g() >> 2) as u8;
            self.buffer[index + 2] = (color.b() >> 3) as u8;
        }
    }

    /// ピクセルを取得
    pub fn get_pixel(&self, x: u32, y: u32) -> Option<Rgb565> {
        if x < self.width && y < self.height {
            let index = ((y * self.width + x) * 3) as usize;
            let r = (self.buffer[index] as u16) << 3;
            let g = (self.buffer[index + 1] as u16) << 2;
            let b = (self.buffer[index + 2] as u16) << 3;
            Some(Rgb565::new(r as u8, g as u8, b as u8))
        } else {
            None
        }
    }

    /// 画面をクリア
    pub fn clear(&mut self, color: Rgb565) {
        for i in (0..self.buffer.len()).step_by(3) {
            self.buffer[i] = (color.r() >> 3) as u8;
            self.buffer[i + 1] = (color.g() >> 2) as u8;
            self.buffer[i + 2] = (color.b() >> 3) as u8;
        }
    }

    /// 矩形を描画
    pub fn draw_rectangle(&mut self, x: u32, y: u32, width: u32, height: u32, color: Rgb565) {
        for dy in 0..height {
            for dx in 0..width {
                self.set_pixel(x + dx, y + dy, color);
            }
        }
    }

    /// 円を描画
    pub fn draw_circle(&mut self, center_x: u32, center_y: u32, radius: u32, color: Rgb565) {
        for dy in 0..(radius * 2) {
            for dx in 0..(radius * 2) {
                let x = dx as i32 - radius as i32;
                let y = dy as i32 - radius as i32;
                if x * x + y * y <= (radius as i32 * radius as i32) {
                    self.set_pixel(
                        (center_x as i32 + x) as u32,
                        (center_y as i32 + y) as u32,
                        color,
                    );
                }
            }
        }
    }

    /// 線を描画
    pub fn draw_line(&mut self, x1: u32, y1: u32, x2: u32, y2: u32, color: Rgb565) {
        let dx = (x2 as i32 - x1 as i32).abs();
        let dy = (y2 as i32 - y1 as i32).abs();
        let sx = if x1 < x2 { 1 } else { -1 };
        let sy = if y1 < y2 { 1 } else { -1 };
        let mut err = dx - dy;

        let mut x = x1 as i32;
        let mut y = y1 as i32;

        loop {
            self.set_pixel(x as u32, y as u32, color);

            if x == x2 as i32 && y == y2 as i32 {
                break;
            }

            let e2 = 2 * err;
            if e2 > -dy {
                err -= dy;
                x += sx;
            }
            if e2 < dx {
                err += dx;
                y += sy;
            }
        }
    }
}

/// グラフィックスシステム
pub struct GraphicsSystem {
    pub context: GraphicsContext,
    pub current_color: Rgb565,
    pub background_color: Rgb565,
}

impl GraphicsSystem {
    /// 新しいグラフィックスシステムを作成
    pub fn new(width: u32, height: u32) -> Self {
        Self {
            context: GraphicsContext::new(width, height),
            current_color: Rgb565::WHITE,
            background_color: Rgb565::BLACK,
        }
    }

    /// 色を設定
    pub fn set_color(&mut self, color: Rgb565) {
        self.current_color = color;
    }

    /// 背景色を設定
    pub fn set_background_color(&mut self, color: Rgb565) {
        self.background_color = color;
    }

    /// 画面をクリア
    pub fn clear(&mut self) {
        self.context.clear(self.background_color);
    }

    /// 矩形を描画
    pub fn draw_rectangle(&mut self, x: u32, y: u32, width: u32, height: u32) {
        self.context.draw_rectangle(x, y, width, height, self.current_color);
    }

    /// 円を描画
    pub fn draw_circle(&mut self, center_x: u32, center_y: u32, radius: u32) {
        self.context.draw_circle(center_x, center_y, radius, self.current_color);
    }

    /// 線を描画
    pub fn draw_line(&mut self, x1: u32, y1: u32, x2: u32, y2: u32) {
        self.context.draw_line(x1, y1, x2, y2, self.current_color);
    }

    /// テキストを描画
    pub fn draw_text(&mut self, x: u32, y: u32, text: &str) {
        // 簡易テキスト描画実装
        // TODO: より高度なテキスト描画を実装
        for (i, ch) in text.chars().enumerate() {
            let char_x = x + (i as u32 * 8);
            if char_x < self.context.width {
                self.draw_character(char_x, y, ch);
            }
        }
    }

    /// 文字を描画
    fn draw_character(&mut self, x: u32, y: u32, ch: char) {
        // 簡易文字描画実装
        // TODO: フォントデータを使用した文字描画を実装
        match ch {
            'A' => {
                self.draw_line(x + 4, y + 8, x, y);
                self.draw_line(x + 4, y + 8, x + 8, y);
                self.draw_line(x + 1, y + 4, x + 7, y + 4);
            },
            'B' => {
                self.draw_line(x, y, x, y + 8);
                self.draw_line(x, y, x + 6, y);
                self.draw_line(x, y + 4, x + 6, y + 4);
                self.draw_line(x, y + 8, x + 6, y + 8);
                self.draw_line(x + 6, y, x + 6, y + 4);
                self.draw_line(x + 6, y + 4, x + 6, y + 8);
            },
            _ => {
                // デフォルト文字（四角）
                self.draw_rectangle(x, y, 8, 8);
            }
        }
    }
}

/// グローバルグラフィックスシステム
static mut GRAPHICS_SYSTEM: Option<GraphicsSystem> = None;

/// グラフィックスシステムの初期化
pub fn init() {
    unsafe {
        GRAPHICS_SYSTEM = Some(GraphicsSystem::new(800, 600));
    }
}

/// グラフィックスシステムを取得
pub fn get_graphics_system() -> Option<&'static mut GraphicsSystem> {
    unsafe {
        GRAPHICS_SYSTEM.as_mut()
    }
}

/// 描画処理
pub fn render() {
    unsafe {
        if let Some(ref mut system) = GRAPHICS_SYSTEM {
            // 描画処理
            // TODO: 実際の描画処理を実装
        }
    }
}


