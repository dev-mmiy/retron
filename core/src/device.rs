//! デバイス抽象化レイヤー

use crate::platform::Platform;
use core::prelude::*;

/// デバイス抽象化トレイト
pub trait Device {
    /// デバイスを初期化
    fn init(&mut self) -> bool;
    
    /// デバイスを有効化
    fn enable(&mut self) -> bool;
    
    /// デバイスを無効化
    fn disable(&mut self) -> bool;
    
    /// デバイス状態を取得
    fn get_status(&self) -> DeviceStatus;
}

/// デバイス状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum DeviceStatus {
    Uninitialized,
    Initialized,
    Enabled,
    Disabled,
    Error,
}

/// デバイス管理
pub struct DeviceManager {
    devices: core::slice::Iter<'static, Box<dyn Device>>,
    platform: Platform,
}

impl DeviceManager {
    /// 新しいデバイスマネージャーを作成
    pub fn new(platform: Platform) -> Self {
        Self {
            devices: [].iter(),
            platform,
        }
    }

    /// デバイスを登録
    pub fn register_device(&mut self, device: Box<dyn Device>) {
        self.devices.push(device);
    }

    /// デバイスを初期化
    pub fn init_all_devices(&mut self) -> bool {
        for device in &mut self.devices {
            if !device.init() {
                return false;
            }
        }
        true
    }

    /// デバイスを有効化
    pub fn enable_all_devices(&mut self) -> bool {
        for device in &mut self.devices {
            if !device.enable() {
                return false;
            }
        }
        true
    }

    /// デバイスを無効化
    pub fn disable_all_devices(&mut self) -> bool {
        for device in &mut self.devices {
            if !device.disable() {
                return false;
            }
        }
        true
    }

    /// デバイス状態を取得
    pub fn get_device_status(&self, index: usize) -> Option<DeviceStatus> {
        if index < self.devices.len() {
            Some(self.devices[index].get_status())
        } else {
            None
        }
    }
}

/// プラットフォーム固有のデバイス管理
pub struct PlatformDeviceManager {
    pub laptop_manager: Option<crate::laptop::LaptopDeviceManager>,
    pub mobile_manager: Option<crate::mobile::MobileDeviceManager>,
    pub common_manager: crate::common::CommonDeviceManager,
}

impl PlatformDeviceManager {
    /// 新しいプラットフォームデバイスマネージャーを作成
    pub fn new(platform: Platform) -> Self {
        Self {
            laptop_manager: if platform == Platform::Laptop {
                Some(crate::laptop::LaptopDeviceManager::new())
            } else {
                None
            },
            mobile_manager: if platform == Platform::Mobile {
                Some(crate::mobile::MobileDeviceManager::new())
            } else {
                None
            },
            common_manager: crate::common::CommonDeviceManager::new(),
        }
    }

    /// デバイスを有効化
    pub fn enable_device(&mut self, device_type: &str) -> bool {
        // 共通デバイスをチェック
        if self.common_manager.enable_device(device_type) {
            return true;
        }

        // プラットフォーム固有デバイスをチェック
        if let Some(ref mut laptop_manager) = self.laptop_manager {
            if laptop_manager.enable_device(device_type) {
                return true;
            }
        }

        if let Some(ref mut mobile_manager) = self.mobile_manager {
            if mobile_manager.enable_device(device_type) {
                return true;
            }
        }

        false
    }

    /// デバイスを無効化
    pub fn disable_device(&mut self, device_type: &str) -> bool {
        // 共通デバイスをチェック
        if self.common_manager.disable_device(device_type) {
            return true;
        }

        // プラットフォーム固有デバイスをチェック
        if let Some(ref mut laptop_manager) = self.laptop_manager {
            if laptop_manager.disable_device(device_type) {
                return true;
            }
        }

        if let Some(ref mut mobile_manager) = self.mobile_manager {
            if mobile_manager.disable_device(device_type) {
                return true;
            }
        }

        false
    }
}
