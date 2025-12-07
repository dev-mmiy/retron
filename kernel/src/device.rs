//! デバイス管理モジュール
//!
//! デバイスドライバーの管理と抽象化

use crate::prelude::*;
use core::sync::atomic::{AtomicUsize, Ordering};

/// デバイスタイプ
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum DeviceType {
    Serial,
    Timer,
    Keyboard,
    Mouse,
    Display,
    Network,
    Storage,
    Sensor,
    Actuator,
}

/// デバイス状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum DeviceState {
    Uninitialized,
    Initialized,
    Active,
    Error,
}

/// デバイス構造体
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Device {
    pub id: usize,
    pub device_type: DeviceType,
    pub state: DeviceState,
    pub name: &'static str,
    pub base_address: usize,
    pub interrupt_number: Option<u8>,
}

/// デバイス管理
pub struct DeviceManager {
    devices: [Option<Device>; 64], // 最大64デバイス
    next_device_id: AtomicUsize,
}

impl Default for DeviceManager {
    fn default() -> Self {
        Self::new()
    }
}

impl DeviceManager {
    /// 新しいデバイスマネージャーを作成
    pub fn new() -> Self {
        Self {
            devices: [None; 64],
            next_device_id: AtomicUsize::new(1),
        }
    }

    /// デバイスを登録
    pub fn register_device(
        &mut self,
        device_type: DeviceType,
        name: &'static str,
        base_address: usize,
        interrupt_number: Option<u8>,
    ) -> Option<usize> {
        let device_id = self.next_device_id.fetch_add(1, Ordering::SeqCst);

        if device_id >= 64 {
            return None;
        }

        let device = Device {
            id: device_id,
            device_type,
            state: DeviceState::Uninitialized,
            name,
            base_address,
            interrupt_number,
        };

        self.devices[device_id] = Some(device);
        Some(device_id)
    }

    /// デバイスを初期化
    pub fn initialize_device(&mut self, device_id: usize) -> bool {
        if let Some(device) = self.devices[device_id].as_mut() {
            device.state = DeviceState::Initialized;
            true
        } else {
            false
        }
    }

    /// デバイスをアクティブ化
    pub fn activate_device(&mut self, device_id: usize) -> bool {
        if let Some(device) = self.devices[device_id].as_mut() {
            if device.state == DeviceState::Initialized {
                device.state = DeviceState::Active;
                true
            } else {
                false
            }
        } else {
            false
        }
    }

    /// デバイスを検索
    pub fn find_device(&self, device_type: DeviceType) -> Option<usize> {
        for (i, device) in self.devices.iter().enumerate() {
            if let Some(device) = device {
                if device.device_type == device_type && device.state == DeviceState::Active {
                    return Some(i);
                }
            }
        }
        None
    }

    /// デバイス一覧を取得
    pub fn get_devices(&self) -> core::slice::Iter<'_, Option<Device>> {
        self.devices.iter()
    }
}

/// グローバルデバイスマネージャー
static mut DEVICE_MANAGER: Option<DeviceManager> = None;

/// デバイス管理の初期化
pub fn init() {
    unsafe {
        DEVICE_MANAGER = Some(DeviceManager::new());

        // 基本的なデバイスを登録
        register_serial_devices();
        register_timer_devices();
    }
}

/// シリアルデバイスの登録
fn register_serial_devices() {
    unsafe {
        if let Some(ref mut manager) = DEVICE_MANAGER {
            // COM1 (0x3F8)
            manager.register_device(DeviceType::Serial, "COM1", 0x3F8, Some(4));
            // COM2 (0x2F8)
            manager.register_device(DeviceType::Serial, "COM2", 0x2F8, Some(3));
        }
    }
}

/// タイマーデバイスの登録
fn register_timer_devices() {
    unsafe {
        if let Some(ref mut manager) = DEVICE_MANAGER {
            // PIT (Programmable Interval Timer)
            manager.register_device(DeviceType::Timer, "PIT", 0x40, Some(0));
        }
    }
}

/// デバイスを登録
pub fn register_device(
    device_type: DeviceType,
    name: &'static str,
    base_address: usize,
    interrupt_number: Option<u8>,
) -> Option<usize> {
    unsafe {
        if let Some(ref mut manager) = DEVICE_MANAGER {
            manager.register_device(device_type, name, base_address, interrupt_number)
        } else {
            None
        }
    }
}

/// デバイスを初期化
pub fn initialize_device(device_id: usize) -> bool {
    unsafe {
        if let Some(ref mut manager) = DEVICE_MANAGER {
            manager.initialize_device(device_id)
        } else {
            false
        }
    }
}

/// デバイスをアクティブ化
pub fn activate_device(device_id: usize) -> bool {
    unsafe {
        if let Some(ref mut manager) = DEVICE_MANAGER {
            manager.activate_device(device_id)
        } else {
            false
        }
    }
}

/// デバイスを検索
pub fn find_device(device_type: DeviceType) -> Option<usize> {
    unsafe {
        if let Some(ref manager) = DEVICE_MANAGER {
            manager.find_device(device_type)
        } else {
            None
        }
    }
}

/// デバイス処理
pub fn process() {
    // デバイスの定期的な処理
    // TODO: 各デバイスの処理を実装
}
