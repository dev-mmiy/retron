//! タスク管理モジュール
//! 
//! μT-Kernel互換のタスク管理機能を提供

use crate::prelude::*;
use core::sync::atomic::{AtomicUsize, Ordering};

/// タスク状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum TaskState {
    Ready,
    Running,
    Waiting,
    Suspended,
    Dormant,
}

/// タスク優先度
pub type TaskPriority = u8;

/// タスクID
pub type TaskId = usize;

/// タスク構造体
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Task {
    pub id: TaskId,
    pub state: TaskState,
    pub priority: TaskPriority,
    pub stack_pointer: usize,
    pub stack_size: usize,
    pub entry_point: fn(),
}

/// タスク管理
pub struct TaskManager {
    tasks: [Option<Task>; 32], // 最大32タスク
    current_task: Option<TaskId>,
    next_task_id: AtomicUsize,
}

impl TaskManager {
    /// 新しいタスクマネージャーを作成
    pub fn new() -> Self {
        Self {
            tasks: [None; 32],
            current_task: None,
            next_task_id: AtomicUsize::new(1),
        }
    }

    /// タスクを作成
    pub fn create_task(&mut self, priority: TaskPriority, stack_size: usize, entry_point: fn()) -> Option<TaskId> {
        let task_id = self.next_task_id.fetch_add(1, Ordering::SeqCst);
        
        if task_id >= 32 {
            return None;
        }

        let task = Task {
            id: task_id,
            state: TaskState::Ready,
            priority,
            stack_pointer: 0, // TODO: スタック領域の割り当て
            stack_size,
            entry_point,
        };

        self.tasks[task_id] = Some(task);
        Some(task_id)
    }

    /// タスクを開始
    pub fn start_task(&mut self, task_id: TaskId) -> bool {
        if let Some(task) = self.tasks[task_id].as_mut() {
            task.state = TaskState::Ready;
            true
        } else {
            false
        }
    }

    /// タスクを終了
    pub fn exit_task(&mut self, task_id: TaskId) -> bool {
        if let Some(task) = self.tasks[task_id].as_mut() {
            task.state = TaskState::Dormant;
            true
        } else {
            false
        }
    }

    /// タスクスケジューリング
    pub fn schedule(&mut self) {
        // 優先度ベースのスケジューリング
        let mut highest_priority = 0;
        let mut next_task = None;

        for (i, task) in self.tasks.iter().enumerate() {
            if let Some(task) = task {
                if task.state == TaskState::Ready && task.priority > highest_priority {
                    highest_priority = task.priority;
                    next_task = Some(i);
                }
            }
        }

        if let Some(task_id) = next_task {
            self.current_task = Some(task_id);
            if let Some(task) = self.tasks[task_id].as_mut() {
                task.state = TaskState::Running;
            }
        }
    }

    /// 現在のタスクを取得
    pub fn get_current_task(&self) -> Option<TaskId> {
        self.current_task
    }
}

/// グローバルタスクマネージャー
static mut TASK_MANAGER: Option<TaskManager> = None;

/// タスク管理の初期化
pub fn init() {
    unsafe {
        TASK_MANAGER = Some(TaskManager::new());
    }
}

/// タスクスケジューラー
pub fn scheduler() {
    unsafe {
        if let Some(ref mut manager) = TASK_MANAGER {
            manager.schedule();
        }
    }
}

/// タスクを作成
pub fn create_task(priority: TaskPriority, stack_size: usize, entry_point: fn()) -> Option<TaskId> {
    unsafe {
        if let Some(ref mut manager) = TASK_MANAGER {
            manager.create_task(priority, stack_size, entry_point)
        } else {
            None
        }
    }
}

/// タスクを開始
pub fn start_task(task_id: TaskId) -> bool {
    unsafe {
        if let Some(ref mut manager) = TASK_MANAGER {
            manager.start_task(task_id)
        } else {
            false
        }
    }
}

/// タスクを終了
pub fn exit_task(task_id: TaskId) -> bool {
    unsafe {
        if let Some(ref mut manager) = TASK_MANAGER {
            manager.exit_task(task_id)
        } else {
            false
        }
    }
}
