//! アクチュエーター制御モジュール

use core::sync::atomic::{AtomicF32, Ordering};

/// アクチュエーター種別
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ActuatorType {
    Motor,
    Servo,
    Stepper,
    Pneumatic,
    Hydraulic,
}

/// アクチュエーター状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ActuatorState {
    Idle,
    Moving,
    Stopped,
    Error,
}

/// アクチュエーター構造体
pub struct Actuator {
    pub id: usize,
    pub actuator_type: ActuatorType,
    pub state: ActuatorState,
    pub position: f32,
    pub velocity: f32,
    pub torque: f32,
    pub max_position: f32,
    pub min_position: f32,
    pub max_velocity: f32,
    pub max_torque: f32,
}

impl Actuator {
    /// 新しいアクチュエーターを作成
    pub fn new(id: usize, actuator_type: ActuatorType) -> Self {
        Self {
            id,
            actuator_type,
            state: ActuatorState::Idle,
            position: 0.0,
            velocity: 0.0,
            torque: 0.0,
            max_position: 360.0,
            min_position: -360.0,
            max_velocity: 100.0,
            max_torque: 10.0,
        }
    }

    /// 位置を設定
    pub fn set_position(&mut self, position: f32) -> bool {
        if position >= self.min_position && position <= self.max_position {
            self.position = position;
            self.state = ActuatorState::Moving;
            true
        } else {
            false
        }
    }

    /// 速度を設定
    pub fn set_velocity(&mut self, velocity: f32) -> bool {
        if velocity.abs() <= self.max_velocity {
            self.velocity = velocity;
            self.state = ActuatorState::Moving;
            true
        } else {
            false
        }
    }

    /// トルクを設定
    pub fn set_torque(&mut self, torque: f32) -> bool {
        if torque.abs() <= self.max_torque {
            self.torque = torque;
            self.state = ActuatorState::Moving;
            true
        } else {
            false
        }
    }

    /// 停止
    pub fn stop(&mut self) {
        self.velocity = 0.0;
        self.torque = 0.0;
        self.state = ActuatorState::Stopped;
    }

    /// 更新
    pub fn update(&mut self, delta_time: f32) {
        match self.state {
            ActuatorState::Moving => {
                // 位置を更新
                self.position += self.velocity * delta_time;
                
                // 制限チェック
                if self.position > self.max_position {
                    self.position = self.max_position;
                    self.state = ActuatorState::Stopped;
                } else if self.position < self.min_position {
                    self.position = self.min_position;
                    self.state = ActuatorState::Stopped;
                }
            },
            _ => {}
        }
    }
}

/// アクチュエーター管理システム
pub struct ActuatorManager {
    actuators: Vec<Actuator>,
    next_actuator_id: AtomicUsize,
}

impl ActuatorManager {
    /// 新しいアクチュエーターマネージャーを作成
    pub fn new() -> Self {
        Self {
            actuators: Vec::new(),
            next_actuator_id: AtomicUsize::new(1),
        }
    }

    /// アクチュエーターを追加
    pub fn add_actuator(&mut self, actuator_type: ActuatorType) -> usize {
        let id = self.next_actuator_id.fetch_add(1, Ordering::SeqCst);
        let actuator = Actuator::new(id, actuator_type);
        self.actuators.push(actuator);
        id
    }

    /// アクチュエーターを取得
    pub fn get_actuator(&mut self, id: usize) -> Option<&mut Actuator> {
        self.actuators.iter_mut().find(|a| a.id == id)
    }

    /// 全アクチュエーターを更新
    pub fn update_all(&mut self, delta_time: f32) {
        for actuator in &mut self.actuators {
            actuator.update(delta_time);
        }
    }

    /// アクチュエーター一覧を取得
    pub fn get_actuators(&self) -> &Vec<Actuator> {
        &self.actuators
    }
}

/// グローバルアクチュエーターマネージャー
static mut ACTUATOR_MANAGER: Option<ActuatorManager> = None;

/// アクチュエーターシステムの初期化
pub fn init() {
    unsafe {
        ACTUATOR_MANAGER = Some(ActuatorManager::new());
    }
}

/// アクチュエーターマネージャーを取得
pub fn get_actuator_manager() -> Option<&'static mut ActuatorManager> {
    unsafe {
        ACTUATOR_MANAGER.as_mut()
    }
}

/// アクチュエーターを追加
pub fn add_actuator(actuator_type: ActuatorType) -> Option<usize> {
    unsafe {
        if let Some(ref mut manager) = ACTUATOR_MANAGER {
            Some(manager.add_actuator(actuator_type))
        } else {
            None
        }
    }
}

/// アクチュエーターを取得
pub fn get_actuator(id: usize) -> Option<&'static mut Actuator> {
    unsafe {
        if let Some(ref mut manager) = ACTUATOR_MANAGER {
            manager.get_actuator(id)
        } else {
            None
        }
    }
}

/// アクチュエーターの位置を設定
pub fn set_actuator_position(id: usize, position: f32) -> bool {
    unsafe {
        if let Some(ref mut manager) = ACTUATOR_MANAGER {
            if let Some(actuator) = manager.get_actuator(id) {
                actuator.set_position(position)
            } else {
                false
            }
        } else {
            false
        }
    }
}

/// アクチュエーターの速度を設定
pub fn set_actuator_velocity(id: usize, velocity: f32) -> bool {
    unsafe {
        if let Some(ref mut manager) = ACTUATOR_MANAGER {
            if let Some(actuator) = manager.get_actuator(id) {
                actuator.set_velocity(velocity)
            } else {
                false
            }
        } else {
            false
        }
    }
}

/// アクチュエーターを停止
pub fn stop_actuator(id: usize) -> bool {
    unsafe {
        if let Some(ref mut manager) = ACTUATOR_MANAGER {
            if let Some(actuator) = manager.get_actuator(id) {
                actuator.stop();
                true
            } else {
                false
            }
        } else {
            false
        }
    }
}

/// 全アクチュエーターを更新
pub fn update_actuators() {
    unsafe {
        if let Some(ref mut manager) = ACTUATOR_MANAGER {
            // デルタタイムの計算（簡易実装）
            let delta_time = 0.016; // 60FPS想定
            manager.update_all(delta_time);
        }
    }
}


