//! センサー制御モジュール

use core::sync::atomic::{AtomicF32, Ordering};

/// センサー種別
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum SensorType {
    Accelerometer,
    Gyroscope,
    Magnetometer,
    GPS,
    Camera,
    Lidar,
    Ultrasonic,
    Infrared,
    Touch,
    Temperature,
    Pressure,
    Humidity,
}

/// センサー状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum SensorState {
    Idle,
    Active,
    Error,
    Calibrating,
}

/// センサー値
#[derive(Debug, Clone, Copy)]
pub struct SensorValue {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub timestamp: u64,
}

impl SensorValue {
    /// 新しいセンサー値を作成
    pub fn new(x: f32, y: f32, z: f32) -> Self {
        Self {
            x,
            y,
            z,
            timestamp: 0, // TODO: 実際のタイムスタンプを設定
        }
    }

    /// マグニチュードを計算
    pub fn magnitude(&self) -> f32 {
        (self.x * self.x + self.y * self.y + self.z * self.z).sqrt()
    }
}

/// センサー構造体
pub struct Sensor {
    pub id: usize,
    pub sensor_type: SensorType,
    pub state: SensorState,
    pub value: SensorValue,
    pub min_value: f32,
    pub max_value: f32,
    pub resolution: f32,
    pub sample_rate: f32,
}

impl Sensor {
    /// 新しいセンサーを作成
    pub fn new(id: usize, sensor_type: SensorType) -> Self {
        Self {
            id,
            sensor_type,
            state: SensorState::Idle,
            value: SensorValue::new(0.0, 0.0, 0.0),
            min_value: -1000.0,
            max_value: 1000.0,
            resolution: 0.1,
            sample_rate: 100.0,
        }
    }

    /// センサー値を設定
    pub fn set_value(&mut self, value: SensorValue) -> bool {
        if value.x >= self.min_value && value.x <= self.max_value &&
           value.y >= self.min_value && value.y <= self.max_value &&
           value.z >= self.min_value && value.z <= self.max_value {
            self.value = value;
            self.state = SensorState::Active;
            true
        } else {
            self.state = SensorState::Error;
            false
        }
    }

    /// センサーを有効化
    pub fn enable(&mut self) {
        self.state = SensorState::Active;
    }

    /// センサーを無効化
    pub fn disable(&mut self) {
        self.state = SensorState::Idle;
    }

    /// センサーを較正
    pub fn calibrate(&mut self) {
        self.state = SensorState::Calibrating;
        // TODO: 実際の較正処理を実装
    }

    /// 更新
    pub fn update(&mut self) {
        match self.state {
            SensorState::Active => {
                // センサーデータの読み取り
                // TODO: 実際のセンサーデータ読み取りを実装
            },
            SensorState::Calibrating => {
                // 較正処理
                // TODO: 実際の較正処理を実装
            },
            _ => {}
        }
    }
}

/// センサー管理システム
pub struct SensorManager {
    sensors: Vec<Sensor>,
    next_sensor_id: AtomicUsize,
}

impl SensorManager {
    /// 新しいセンサーマネージャーを作成
    pub fn new() -> Self {
        Self {
            sensors: Vec::new(),
            next_sensor_id: AtomicUsize::new(1),
        }
    }

    /// センサーを追加
    pub fn add_sensor(&mut self, sensor_type: SensorType) -> usize {
        let id = self.next_sensor_id.fetch_add(1, Ordering::SeqCst);
        let sensor = Sensor::new(id, sensor_type);
        self.sensors.push(sensor);
        id
    }

    /// センサーを取得
    pub fn get_sensor(&mut self, id: usize) -> Option<&mut Sensor> {
        self.sensors.iter_mut().find(|s| s.id == id)
    }

    /// センサータイプで検索
    pub fn find_sensor_by_type(&mut self, sensor_type: SensorType) -> Option<&mut Sensor> {
        self.sensors.iter_mut().find(|s| s.sensor_type == sensor_type)
    }

    /// 全センサーを更新
    pub fn update_all(&mut self) {
        for sensor in &mut self.sensors {
            sensor.update();
        }
    }

    /// センサー一覧を取得
    pub fn get_sensors(&self) -> &Vec<Sensor> {
        &self.sensors
    }
}

/// グローバルセンサーマネージャー
static mut SENSOR_MANAGER: Option<SensorManager> = None;

/// センサーシステムの初期化
pub fn init() {
    unsafe {
        SENSOR_MANAGER = Some(SensorManager::new());
    }
}

/// センサーマネージャーを取得
pub fn get_sensor_manager() -> Option<&'static mut SensorManager> {
    unsafe {
        SENSOR_MANAGER.as_mut()
    }
}

/// センサーを追加
pub fn add_sensor(sensor_type: SensorType) -> Option<usize> {
    unsafe {
        if let Some(ref mut manager) = SENSOR_MANAGER {
            Some(manager.add_sensor(sensor_type))
        } else {
            None
        }
    }
}

/// センサーを取得
pub fn get_sensor(id: usize) -> Option<&'static mut Sensor> {
    unsafe {
        if let Some(ref mut manager) = SENSOR_MANAGER {
            manager.get_sensor(id)
        } else {
            None
        }
    }
}

/// センサータイプで検索
pub fn find_sensor_by_type(sensor_type: SensorType) -> Option<&'static mut Sensor> {
    unsafe {
        if let Some(ref mut manager) = SENSOR_MANAGER {
            manager.find_sensor_by_type(sensor_type)
        } else {
            None
        }
    }
}

/// センサー値を設定
pub fn set_sensor_value(id: usize, value: SensorValue) -> bool {
    unsafe {
        if let Some(ref mut manager) = SENSOR_MANAGER {
            if let Some(sensor) = manager.get_sensor(id) {
                sensor.set_value(value)
            } else {
                false
            }
        } else {
            false
        }
    }
}

/// センサーを有効化
pub fn enable_sensor(id: usize) -> bool {
    unsafe {
        if let Some(ref mut manager) = SENSOR_MANAGER {
            if let Some(sensor) = manager.get_sensor(id) {
                sensor.enable();
                true
            } else {
                false
            }
        } else {
            false
        }
    }
}

/// センサーを無効化
pub fn disable_sensor(id: usize) -> bool {
    unsafe {
        if let Some(ref mut manager) = SENSOR_MANAGER {
            if let Some(sensor) = manager.get_sensor(id) {
                sensor.disable();
                true
            } else {
                false
            }
        } else {
            false
        }
    }
}

/// 全センサーを更新
pub fn update_sensors() {
    unsafe {
        if let Some(ref mut manager) = SENSOR_MANAGER {
            manager.update_all();
        }
    }
}


