//! Retron Core
//! 
//! デバイス抽象化レイヤーとプラットフォーム固有の機能

#![no_std]

pub mod device;
pub mod platform;
pub mod laptop;
pub mod mobile;
pub mod common;

/// Coreレイヤーの初期化
pub fn init() {
    // プラットフォーム検出
    let platform = platform::detect_platform();
    
    match platform {
        platform::Platform::Laptop => {
            laptop::init();
        },
        platform::Platform::Mobile => {
            mobile::init();
        },
        platform::Platform::Unknown => {
            // 汎用初期化
            common::init();
        }
    }
}

/// プラットフォーム固有の機能を取得
pub fn get_platform_features() -> Vec<&'static str> {
    let platform = platform::detect_platform();
    
    match platform {
        platform::Platform::Laptop => laptop::get_features(),
        platform::Platform::Mobile => mobile::get_features(),
        platform::Platform::Unknown => common::get_features(),
    }
}
