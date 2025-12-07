//! μT-Kernel互換レイヤー
//!
//! μT-Kernel 3.x APIの互換性を提供

use crate::prelude::*;
use crate::task::{TaskId, TaskPriority};

/// μT-Kernel互換のタスク管理
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct UTKernelTask {
    pub tskid: TaskId,
    pub itskpri: TaskPriority,
    pub stksz: usize,
    pub stk: *mut u8,
}

/// μT-Kernel互換のメモリ管理
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct UTKernelMemory {
    pub mpfid: usize,
    pub blksz: usize,
    pub blkcnt: usize,
    pub mpf: *mut u8,
}

/// μT-Kernel互換レイヤー
pub struct UTKernelLayer {
    tasks: [Option<UTKernelTask>; 32],
    memory_pools: [Option<UTKernelMemory>; 16],
    next_task_id: usize,
    next_pool_id: usize,
}

impl Default for UTKernelLayer {
    fn default() -> Self {
        Self::new()
    }
}

impl UTKernelLayer {
    /// 新しいμT-Kernel互換レイヤーを作成
    pub fn new() -> Self {
        Self {
            tasks: [None; 32],
            memory_pools: [None; 16],
            next_task_id: 0,
            next_pool_id: 0,
        }
    }

    /// タスクを作成 (tk_cre_tsk互換)
    pub fn create_task(
        &mut self,
        priority: TaskPriority,
        stack_size: usize,
        _entry_point: fn(),
    ) -> Option<TaskId> {
        // スタック領域の割り当て
        let stack: *mut u8 = core::ptr::null_mut();

        if stack.is_null() {
            return None;
        }

        // 空きスロットを探す
        for i in 0..32 {
            if self.tasks[i].is_none() {
                let task = UTKernelTask {
                    tskid: self.next_task_id,
                    itskpri: priority,
                    stksz: stack_size,
                    stk: stack,
                };

                self.tasks[i] = Some(task);
                self.next_task_id += 1;
                return Some(task.tskid);
            }
        }
        None
    }

    /// タスクを開始 (tk_sta_tsk互換)
    pub fn start_task(&mut self, task_id: TaskId) -> bool {
        for i in 0..32 {
            if let Some(task) = &self.tasks[i] {
                if task.tskid == task_id {
                    // タスクの開始処理
                    return true;
                }
            }
        }
        false
    }

    /// タスクを終了 (tk_ext_tsk互換)
    pub fn exit_task(&mut self, task_id: TaskId) -> bool {
        for i in 0..32 {
            if let Some(task) = &self.tasks[i] {
                if task.tskid == task_id {
                    // スタック領域の解放
                    // スタック領域の解放（簡易実装）
                    // unsafe {
                    //     core::alloc::dealloc(
                    //         task.stk,
                    //         Layout::from_size_align(task.stksz, 16).unwrap()
                    //     );
                    // }
                    self.tasks[i] = None;
                    return true;
                }
            }
        }
        false
    }

    /// メモリプールを作成 (tk_cre_mpf互換)
    pub fn create_memory_pool(&mut self, block_size: usize, block_count: usize) -> Option<usize> {
        let _total_size = block_size * block_count;
        let memory: *mut u8 = core::ptr::null_mut();

        if memory.is_null() {
            return None;
        }

        // 空きスロットを探す
        for i in 0..16 {
            if self.memory_pools[i].is_none() {
                let pool = UTKernelMemory {
                    mpfid: self.next_pool_id,
                    blksz: block_size,
                    blkcnt: block_count,
                    mpf: memory,
                };

                self.memory_pools[i] = Some(pool);
                self.next_pool_id += 1;
                return Some(pool.mpfid);
            }
        }
        None
    }

    /// メモリブロックを取得 (tk_get_mpf互換)
    pub fn get_memory_block(&mut self, pool_id: usize) -> Option<*mut u8> {
        for i in 0..16 {
            if let Some(pool) = &self.memory_pools[i] {
                if pool.mpfid == pool_id {
                    // 簡易実装：実際の実装ではより複雑な管理が必要
                    return Some(pool.mpf);
                }
            }
        }
        None
    }

    /// メモリブロックを返却 (tk_rel_mpf互換)
    pub fn release_memory_block(&mut self, _pool_id: usize, _block: *mut u8) -> bool {
        // 簡易実装：実際の実装ではより複雑な管理が必要
        true
    }
}

/// グローバルμT-Kernel互換レイヤー
static mut UTKERNEL_LAYER: Option<UTKernelLayer> = None;

/// μT-Kernel互換レイヤーの初期化
pub fn init() {
    unsafe {
        UTKERNEL_LAYER = Some(UTKernelLayer::new());
    }
}

/// タスクを作成 (tk_cre_tsk互換)
pub fn tk_cre_tsk(priority: TaskPriority, stack_size: usize, entry_point: fn()) -> Option<TaskId> {
    unsafe {
        if let Some(ref mut layer) = UTKERNEL_LAYER {
            layer.create_task(priority, stack_size, entry_point)
        } else {
            None
        }
    }
}

/// タスクを開始 (tk_sta_tsk互換)
pub fn tk_sta_tsk(task_id: TaskId) -> bool {
    unsafe {
        if let Some(ref mut layer) = UTKERNEL_LAYER {
            layer.start_task(task_id)
        } else {
            false
        }
    }
}

/// タスクを終了 (tk_ext_tsk互換)
pub fn tk_ext_tsk(task_id: TaskId) -> bool {
    unsafe {
        if let Some(ref mut layer) = UTKERNEL_LAYER {
            layer.exit_task(task_id)
        } else {
            false
        }
    }
}

/// メモリプールを作成 (tk_cre_mpf互換)
pub fn tk_cre_mpf(block_size: usize, block_count: usize) -> Option<usize> {
    unsafe {
        if let Some(ref mut layer) = UTKERNEL_LAYER {
            layer.create_memory_pool(block_size, block_count)
        } else {
            None
        }
    }
}

/// メモリブロックを取得 (tk_get_mpf互換)
pub fn tk_get_mpf(pool_id: usize) -> Option<*mut u8> {
    unsafe {
        if let Some(ref mut layer) = UTKERNEL_LAYER {
            layer.get_memory_block(pool_id)
        } else {
            None
        }
    }
}

/// メモリブロックを返却 (tk_rel_mpf互換)
pub fn tk_rel_mpf(pool_id: usize, block: *mut u8) -> bool {
    unsafe {
        if let Some(ref mut layer) = UTKERNEL_LAYER {
            layer.release_memory_block(pool_id, block)
        } else {
            false
        }
    }
}
