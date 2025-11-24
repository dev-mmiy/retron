//! ナビゲーション制御モジュール

use crate::motion::{Position, Orientation};
use core::sync::atomic::{AtomicF32, Ordering};

/// ナビゲーション状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum NavigationState {
    Idle,
    Planning,
    Following,
    Reached,
    Error,
}

/// 経路点
#[derive(Debug, Clone, Copy)]
pub struct Waypoint {
    pub position: Position,
    pub orientation: Orientation,
    pub speed: f32,
}

impl Waypoint {
    /// 新しい経路点を作成
    pub fn new(position: Position, orientation: Orientation, speed: f32) -> Self {
        Self {
            position,
            orientation,
            speed,
        }
    }
}

/// 経路
pub struct Path {
    pub waypoints: Vec<Waypoint>,
    pub current_index: usize,
    pub loop_path: bool,
}

impl Path {
    /// 新しい経路を作成
    pub fn new() -> Self {
        Self {
            waypoints: Vec::new(),
            current_index: 0,
            loop_path: false,
        }
    }

    /// 経路点を追加
    pub fn add_waypoint(&mut self, waypoint: Waypoint) {
        self.waypoints.push(waypoint);
    }

    /// 次の経路点を取得
    pub fn get_next_waypoint(&mut self) -> Option<Waypoint> {
        if self.current_index < self.waypoints.len() {
            let waypoint = self.waypoints[self.current_index];
            self.current_index += 1;
            Some(waypoint)
        } else if self.loop_path && !self.waypoints.is_empty() {
            self.current_index = 0;
            Some(self.waypoints[0])
        } else {
            None
        }
    }

    /// 経路をリセット
    pub fn reset(&mut self) {
        self.current_index = 0;
    }

    /// 経路が完了したかチェック
    pub fn is_complete(&self) -> bool {
        self.current_index >= self.waypoints.len() && !self.loop_path
    }
}

/// ナビゲーションシステム
pub struct NavigationSystem {
    pub state: NavigationState,
    pub current_path: Option<Path>,
    pub current_waypoint: Option<Waypoint>,
    pub goal_position: Option<Position>,
    pub goal_orientation: Option<Orientation>,
    pub max_speed: f32,
    pub max_acceleration: f32,
}

impl NavigationSystem {
    /// 新しいナビゲーションシステムを作成
    pub fn new() -> Self {
        Self {
            state: NavigationState::Idle,
            current_path: None,
            current_waypoint: None,
            goal_position: None,
            goal_orientation: None,
            max_speed: 1.0,
            max_acceleration: 0.5,
        }
    }

    /// 目標位置を設定
    pub fn set_goal(&mut self, position: Position, orientation: Option<Orientation>) {
        self.goal_position = Some(position);
        self.goal_orientation = orientation;
        self.state = NavigationState::Planning;
    }

    /// 経路を設定
    pub fn set_path(&mut self, path: Path) {
        self.current_path = Some(path);
        self.state = NavigationState::Following;
    }

    /// ナビゲーションを開始
    pub fn start(&mut self) {
        if self.current_path.is_some() {
            self.state = NavigationState::Following;
        }
    }

    /// ナビゲーションを停止
    pub fn stop(&mut self) {
        self.state = NavigationState::Idle;
        self.current_path = None;
        self.current_waypoint = None;
        self.goal_position = None;
        self.goal_orientation = None;
    }

    /// 更新
    pub fn update(&mut self) {
        match self.state {
            NavigationState::Planning => {
                self.plan_path();
            },
            NavigationState::Following => {
                self.follow_path();
            },
            _ => {}
        }
    }

    /// 経路計画
    fn plan_path(&mut self) {
        if let Some(goal) = self.goal_position {
            // 簡易経路計画：直線経路
            let mut path = Path::new();
            
            // 現在位置を取得（簡易実装）
            let current_pos = Position::new(0.0, 0.0, 0.0);
            
            // 目標位置への経路点を作成
            let waypoint = Waypoint::new(
                goal,
                self.goal_orientation.unwrap_or(Orientation::new(0.0, 0.0, 0.0)),
                self.max_speed,
            );
            path.add_waypoint(waypoint);
            
            self.current_path = Some(path);
            self.state = NavigationState::Following;
        }
    }

    /// 経路追従
    fn follow_path(&mut self) {
        if let Some(ref mut path) = self.current_path {
            if let Some(waypoint) = path.get_next_waypoint() {
                self.current_waypoint = Some(waypoint);
                
                // モーション制御に目標を設定
                crate::motion::set_target_position(
                    waypoint.position.x,
                    waypoint.position.y,
                    waypoint.position.z,
                );
                crate::motion::set_target_orientation(
                    waypoint.orientation.roll,
                    waypoint.orientation.pitch,
                    waypoint.orientation.yaw,
                );
            } else {
                // 経路完了
                self.state = NavigationState::Reached;
                self.current_waypoint = None;
            }
        }
    }
}

/// グローバルナビゲーションシステム
static mut NAVIGATION_SYSTEM: Option<NavigationSystem> = None;

/// ナビゲーションシステムの初期化
pub fn init() {
    unsafe {
        NAVIGATION_SYSTEM = Some(NavigationSystem::new());
    }
}

/// ナビゲーションシステムを取得
pub fn get_navigation_system() -> Option<&'static mut NavigationSystem> {
    unsafe {
        NAVIGATION_SYSTEM.as_mut()
    }
}

/// 目標位置を設定
pub fn set_goal(x: f32, y: f32, z: f32, roll: Option<f32>, pitch: Option<f32>, yaw: Option<f32>) {
    unsafe {
        if let Some(ref mut system) = NAVIGATION_SYSTEM {
            let position = Position::new(x, y, z);
            let orientation = if let (Some(r), Some(p), Some(y)) = (roll, pitch, yaw) {
                Some(Orientation::new(r, p, y))
            } else {
                None
            };
            system.set_goal(position, orientation);
        }
    }
}

/// 経路を設定
pub fn set_path(waypoints: Vec<(f32, f32, f32, f32, f32, f32, f32)>) {
    unsafe {
        if let Some(ref mut system) = NAVIGATION_SYSTEM {
            let mut path = Path::new();
            for (x, y, z, roll, pitch, yaw, speed) in waypoints {
                let waypoint = Waypoint::new(
                    Position::new(x, y, z),
                    Orientation::new(roll, pitch, yaw),
                    speed,
                );
                path.add_waypoint(waypoint);
            }
            system.set_path(path);
        }
    }
}

/// ナビゲーションを開始
pub fn start_navigation() {
    unsafe {
        if let Some(ref mut system) = NAVIGATION_SYSTEM {
            system.start();
        }
    }
}

/// ナビゲーションを停止
pub fn stop_navigation() {
    unsafe {
        if let Some(ref mut system) = NAVIGATION_SYSTEM {
            system.stop();
        }
    }
}

/// 現在のナビゲーション状態を取得
pub fn get_navigation_state() -> Option<NavigationState> {
    unsafe {
        if let Some(ref system) = NAVIGATION_SYSTEM {
            Some(system.state)
        } else {
            None
        }
    }
}

/// ナビゲーションシステムを更新
pub fn update_navigation() {
    unsafe {
        if let Some(ref mut system) = NAVIGATION_SYSTEM {
            system.update();
        }
    }
}


