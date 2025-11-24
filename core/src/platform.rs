//! プラットフォーム検出と管理

use core::sync::atomic::{AtomicU8, Ordering};

/// プラットフォーム種別
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Platform {
    Laptop,
    Mobile,
    Unknown,
}

/// プラットフォーム情報
pub struct PlatformInfo {
    pub platform: Platform,
    pub cpu_arch: &'static str,
    pub memory_size: usize,
    pub has_graphics: bool,
    pub has_network: bool,
    pub has_sensors: bool,
}

/// プラットフォーム検出
pub fn detect_platform() -> Platform {
    // CPU情報からプラットフォームを判定
    let cpu_info = get_cpu_info();
    
    if cpu_info.contains("x86_64") || cpu_info.contains("x86") {
        // x86系は主にLaptop
        Platform::Laptop
    } else if cpu_info.contains("arm") || cpu_info.contains("aarch64") {
        // ARM系は主にMobile
        Platform::Mobile
    } else {
        Platform::Unknown
    }
}

/// CPU情報を取得
fn get_cpu_info() -> &'static str {
    // 簡易実装：実際の実装ではCPUID命令などを使用
    "x86_64"
}

/// プラットフォーム情報を取得
pub fn get_platform_info() -> PlatformInfo {
    let platform = detect_platform();
    
    match platform {
        Platform::Laptop => PlatformInfo {
            platform,
            cpu_arch: "x86_64",
            memory_size: 8 * 1024 * 1024 * 1024, // 8GB
            has_graphics: true,
            has_network: true,
            has_sensors: false,
        },
        Platform::Mobile => PlatformInfo {
            platform,
            cpu_arch: "aarch64",
            memory_size: 4 * 1024 * 1024 * 1024, // 4GB
            has_graphics: true,
            has_network: true,
            has_sensors: true,
        },
        Platform::Unknown => PlatformInfo {
            platform,
            cpu_arch: "unknown",
            memory_size: 1 * 1024 * 1024 * 1024, // 1GB
            has_graphics: false,
            has_network: false,
            has_sensors: false,
        }
    }
}

/// プラットフォーム固有の設定
pub struct PlatformConfig {
    pub enable_graphics: bool,
    pub enable_network: bool,
    pub enable_sensors: bool,
    pub power_management: bool,
}

impl PlatformConfig {
    /// デフォルト設定を取得
    pub fn default_for_platform(platform: Platform) -> Self {
        match platform {
            Platform::Laptop => Self {
                enable_graphics: true,
                enable_network: true,
                enable_sensors: false,
                power_management: true,
            },
            Platform::Mobile => Self {
                enable_graphics: true,
                enable_network: true,
                enable_sensors: true,
                power_management: true,
            },
            Platform::Unknown => Self {
                enable_graphics: false,
                enable_network: false,
                enable_sensors: false,
                power_management: false,
            }
        }
    }
}


