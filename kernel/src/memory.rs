//! メモリ管理モジュール
//!
//! μT-Kernel互換のメモリ管理機能を提供

use crate::prelude::*;
use linked_list_allocator::LockedHeap;
use spin::Mutex;

extern crate alloc;

/// ヒープサイズ（4MB）
const HEAP_SIZE: usize = 4 * 1024 * 1024;

/// ヒープ領域
static mut HEAP: [u8; HEAP_SIZE] = [0; HEAP_SIZE];

/// グローバルアロケーター
#[global_allocator]
static ALLOCATOR: LockedHeap = LockedHeap::empty();

/// メモリ管理の初期化
pub fn init() {
    unsafe {
        let heap_start = core::ptr::addr_of_mut!(HEAP) as *mut u8;
        let heap_size = HEAP_SIZE;
        ALLOCATOR.lock().init(heap_start, heap_size);
    }
}

/// カスタムアロケーター
pub struct RetronAllocator;

unsafe impl GlobalAlloc for RetronAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        ALLOCATOR.alloc(layout)
    }

    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        ALLOCATOR.dealloc(ptr, layout);
    }
}

/// メモリプール管理
pub struct MemoryPool {
    base_addr: usize,
    size: usize,
    used: usize,
}

impl MemoryPool {
    /// 新しいメモリプールを作成
    pub fn new(base_addr: usize, size: usize) -> Self {
        Self {
            base_addr,
            size,
            used: 0,
        }
    }

    /// メモリを割り当て
    pub fn allocate(&mut self, size: usize) -> Option<usize> {
        if self.used + size <= self.size {
            let addr = self.base_addr + self.used;
            self.used += size;
            Some(addr)
        } else {
            None
        }
    }

    /// メモリを解放
    pub fn deallocate(&mut self, _addr: usize, _size: usize) {
        // 簡易実装：実際の実装ではより複雑な管理が必要
    }
}

/// システムメモリ情報
pub struct SystemMemory {
    pub total: usize,
    pub used: usize,
    pub free: usize,
}

impl SystemMemory {
    /// システムメモリ情報を取得
    pub fn get_info() -> Self {
        let used = get_heap_usage();
        Self {
            total: HEAP_SIZE,
            used,
            free: HEAP_SIZE.saturating_sub(used),
        }
    }
}

/// ヒープ使用量を取得
pub fn get_heap_usage() -> usize {
    ALLOCATOR.lock().used()
}

/// デフォルトスタックサイズ（64KB）
const DEFAULT_STACK_SIZE: usize = 64 * 1024;

/// 最大スタック数
const MAX_STACKS: usize = 32;

/// スタック情報
#[derive(Debug, Clone, Copy, PartialEq)]
struct StackInfo {
    base_addr: usize,
    size: usize,
    in_use: bool,
    guard_page: bool,
}

/// スタックアロケータ
pub struct StackAllocator {
    stacks: [Option<StackInfo>; MAX_STACKS],
    next_id: usize,
}

impl Default for StackAllocator {
    fn default() -> Self {
        Self::new()
    }
}

impl StackAllocator {
    /// 新しいスタックアロケータを作成
    pub const fn new() -> Self {
        Self {
            stacks: [None; MAX_STACKS],
            next_id: 0,
        }
    }

    /// スタックを割り当て
    pub fn allocate_stack(&mut self, size: usize) -> Option<usize> {
        // サイズをページ境界にアラインメント（4KBアライン）
        let aligned_size = (size + 4095) & !4095;

        // ヒープから領域を確保
        let layout = Layout::from_size_align(aligned_size, 4096).ok()?;
        let ptr = unsafe { alloc::alloc::alloc(layout) };

        if ptr.is_null() {
            return None;
        }

        let base_addr = ptr as usize;

        // スタック情報を登録
        for (id, slot) in self.stacks.iter_mut().enumerate() {
            if slot.is_none() {
                *slot = Some(StackInfo {
                    base_addr,
                    size: aligned_size,
                    in_use: true,
                    guard_page: false,
                });
                self.next_id = id + 1;
                // スタックは上位アドレスから下位アドレスに成長するため、
                // スタックポインタは base_addr + size を返す
                return Some(base_addr + aligned_size);
            }
        }

        // スロットがない場合はメモリを解放
        unsafe {
            alloc::alloc::dealloc(ptr, layout);
        }
        None
    }

    /// スタックを解放
    pub fn deallocate_stack(&mut self, stack_pointer: usize) -> bool {
        // スタックポインタからベースアドレスを逆算して検索
        for slot in self.stacks.iter_mut() {
            if let Some(ref info) = slot {
                if info.in_use
                    && stack_pointer > info.base_addr
                    && stack_pointer <= info.base_addr + info.size
                {
                    // メモリを解放
                    let layout = Layout::from_size_align(info.size, 4096).unwrap();
                    unsafe {
                        alloc::alloc::dealloc(info.base_addr as *mut u8, layout);
                    }
                    *slot = None;
                    return true;
                }
            }
        }
        false
    }

    /// ガードページを有効化（将来の拡張用）
    pub fn enable_guard_page(&mut self, stack_pointer: usize) -> bool {
        for info in self.stacks.iter_mut().flatten() {
            if info.in_use
                && stack_pointer > info.base_addr
                && stack_pointer <= info.base_addr + info.size
            {
                info.guard_page = true;
                return true;
            }
        }
        false
    }

    /// スタック使用状況を取得
    pub fn get_stack_usage(&self) -> (usize, usize) {
        let mut in_use = 0;
        let mut total_size = 0;

        for info in self.stacks.iter().flatten() {
            if info.in_use {
                in_use += 1;
                total_size += info.size;
            }
        }

        (in_use, total_size)
    }
}

/// グローバルスタックアロケータ
static STACK_ALLOCATOR: Mutex<StackAllocator> = Mutex::new(StackAllocator::new());

/// スタックを割り当て（グローバルAPI）
pub fn allocate_stack(size: usize) -> Option<usize> {
    STACK_ALLOCATOR.lock().allocate_stack(size)
}

/// デフォルトサイズでスタックを割り当て
pub fn allocate_default_stack() -> Option<usize> {
    allocate_stack(DEFAULT_STACK_SIZE)
}

/// スタックを解放（グローバルAPI）
pub fn deallocate_stack(stack_pointer: usize) -> bool {
    STACK_ALLOCATOR.lock().deallocate_stack(stack_pointer)
}

/// スタック使用状況を取得（グローバルAPI）
pub fn get_stack_usage() -> (usize, usize) {
    STACK_ALLOCATOR.lock().get_stack_usage()
}
