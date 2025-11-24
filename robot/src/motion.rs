//! モーション制御モジュール

use core::sync::atomic::{AtomicF32, Ordering};

/// モーション状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum MotionState {
    Idle,
    Moving,
    Stopped,
    Error,
}

/// 位置
#[derive(Debug, Clone, Copy)]
pub struct Position {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

impl Position {
    /// 新しい位置を作成
    pub fn new(x: f32, y: f32, z: f32) -> Self {
        Self { x, y, z }
    }

    /// 距離を計算
    pub fn distance_to(&self, other: &Position) -> f32 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        let dz = self.z - other.z;
        (dx * dx + dy * dy + dz * dz).sqrt()
    }
}

/// 姿勢
#[derive(Debug, Clone, Copy)]
pub struct Orientation {
    pub roll: f32,
    pub pitch: f32,
    pub yaw: f32,
}

impl Orientation {
    /// 新しい姿勢を作成
    pub fn new(roll: f32, pitch: f32, yaw: f32) -> Self {
        Self { roll, pitch, yaw }
    }
}

/// 速度
#[derive(Debug, Clone, Copy)]
pub struct Velocity {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

impl Velocity {
    /// 新しい速度を作成
    pub fn new(x: f32, y: f32, z: f32) -> Self {
        Self { x, y, z }
    }

    /// マグニチュードを計算
    pub fn magnitude(&self) -> f32 {
        (self.x * self.x + self.y * self.y + self.z * self.z).sqrt()
    }
}

/// モーション制御システム
pub struct MotionController {
    pub position: Position,
    pub orientation: Orientation,
    pub velocity: Velocity,
    pub state: MotionState,
    pub target_position: Option<Position>,
    pub target_orientation: Option<Orientation>,
    pub max_velocity: f32,
    pub max_acceleration: f32,
}

impl MotionController {
    /// 新しいモーションコントローラーを作成
    pub fn new() -> Self {
        Self {
            position: Position::new(0.0, 0.0, 0.0),
            orientation: Orientation::new(0.0, 0.0, 0.0),
            velocity: Velocity::new(0.0, 0.0, 0.0),
            state: MotionState::Idle,
            target_position: None,
            target_orientation: None,
            max_velocity: 1.0,
            max_acceleration: 0.5,
        }
    }

    /// 目標位置を設定
    pub fn set_target_position(&mut self, position: Position) {
        self.target_position = Some(position);
        self.state = MotionState::Moving;
    }

    /// 目標姿勢を設定
    pub fn set_target_orientation(&mut self, orientation: Orientation) {
        self.target_orientation = Some(orientation);
        self.state = MotionState::Moving;
    }

    /// 移動を停止
    pub fn stop(&mut self) {
        self.target_position = None;
        self.target_orientation = None;
        self.velocity = Velocity::new(0.0, 0.0, 0.0);
        self.state = MotionState::Stopped;
    }

    /// 更新
    pub fn update(&mut self, delta_time: f32) {
        match self.state {
            MotionState::Moving => {
                self.update_motion(delta_time);
            },
            _ => {}
        }
    }

    /// モーション更新
    fn update_motion(&mut self, delta_time: f32) {
        // 位置制御
        if let Some(target) = self.target_position {
            let dx = target.x - self.position.x;
            let dy = target.y - self.position.y;
            let dz = target.z - self.position.z;
            let distance = (dx * dx + dy * dy + dz * dz).sqrt();

            if distance > 0.01 {
                // 目標に向かって移動
                let direction_x = dx / distance;
                let direction_y = dy / distance;
                let direction_z = dz / distance;

                let target_velocity = self.max_velocity.min(distance / delta_time);
                self.velocity.x = direction_x * target_velocity;
                self.velocity.y = direction_y * target_velocity;
                self.velocity.z = direction_z * target_velocity;

                // 位置を更新
                self.position.x += self.velocity.x * delta_time;
                self.position.y += self.velocity.y * delta_time;
                self.position.z += self.velocity.z * delta_time;
            } else {
                // 目標に到達
                self.position = target;
                self.velocity = Velocity::new(0.0, 0.0, 0.0);
                self.target_position = None;
                if self.target_orientation.is_none() {
                    self.state = MotionState::Idle;
                }
            }
        }

        // 姿勢制御
        if let Some(target) = self.target_orientation {
            let roll_diff = target.roll - self.orientation.roll;
            let pitch_diff = target.pitch - self.orientation.pitch;
            let yaw_diff = target.yaw - self.orientation.yaw;

            if roll_diff.abs() > 0.01 || pitch_diff.abs() > 0.01 || yaw_diff.abs() > 0.01 {
                // 目標姿勢に向かって回転
                let rotation_speed = 1.0; // rad/s
                self.orientation.roll += roll_diff.signum() * rotation_speed * delta_time;
                self.orientation.pitch += pitch_diff.signum() * rotation_speed * delta_time;
                self.orientation.yaw += yaw_diff.signum() * rotation_speed * delta_time;
            } else {
                // 目標姿勢に到達
                self.orientation = target;
                self.target_orientation = None;
                if self.target_position.is_none() {
                    self.state = MotionState::Idle;
                }
            }
        }
    }
}

/// グローバルモーションコントローラー
static mut MOTION_CONTROLLER: Option<MotionController> = None;

/// モーション制御システムの初期化
pub fn init() {
    unsafe {
        MOTION_CONTROLLER = Some(MotionController::new());
    }
}

/// モーションコントローラーを取得
pub fn get_motion_controller() -> Option<&'static mut MotionController> {
    unsafe {
        MOTION_CONTROLLER.as_mut()
    }
}

/// 目標位置を設定
pub fn set_target_position(x: f32, y: f32, z: f32) {
    unsafe {
        if let Some(ref mut controller) = MOTION_CONTROLLER {
            controller.set_target_position(Position::new(x, y, z));
        }
    }
}

/// 目標姿勢を設定
pub fn set_target_orientation(roll: f32, pitch: f32, yaw: f32) {
    unsafe {
        if let Some(ref mut controller) = MOTION_CONTROLLER {
            controller.set_target_orientation(Orientation::new(roll, pitch, yaw));
        }
    }
}

/// 移動を停止
pub fn stop_motion() {
    unsafe {
        if let Some(ref mut controller) = MOTION_CONTROLLER {
            controller.stop();
        }
    }
}

/// 現在位置を取得
pub fn get_position() -> Option<Position> {
    unsafe {
        if let Some(ref controller) = MOTION_CONTROLLER {
            Some(controller.position)
        } else {
            None
        }
    }
}

/// 現在姿勢を取得
pub fn get_orientation() -> Option<Orientation> {
    unsafe {
        if let Some(ref controller) = MOTION_CONTROLLER {
            Some(controller.orientation)
        } else {
            None
        }
    }
}

/// 現在速度を取得
pub fn get_velocity() -> Option<Velocity> {
    unsafe {
        if let Some(ref controller) = MOTION_CONTROLLER {
            Some(controller.velocity)
        } else {
            None
        }
    }
}

/// モーション制御を更新
pub fn update_motion() {
    unsafe {
        if let Some(ref mut controller) = MOTION_CONTROLLER {
            let delta_time = 0.016; // 60FPS想定
            controller.update(delta_time);
        }
    }
}


