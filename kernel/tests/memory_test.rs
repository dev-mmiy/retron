//! メモリ管理機能のテスト
//!
//! Step 1で実装したメモリ管理機能の動作確認

use retron_kernel::memory;

/// メモリ使用量取得のテスト
pub fn test_heap_usage() -> bool {
    let usage = memory::get_heap_usage();
    // ヒープは初期化されているので、0以上のはず
    usage >= 0
}

/// SystemMemory::get_info()のテスト
pub fn test_system_memory_info() -> bool {
    let info = memory::SystemMemory::get_info();

    // 検証項目
    let valid_total = info.total == 4 * 1024 * 1024; // 4MB
    let valid_used = info.used >= 0 && info.used <= info.total;
    let valid_free = info.free >= 0 && info.free <= info.total;
    let valid_sum = info.used + info.free <= info.total;

    valid_total && valid_used && valid_free && valid_sum
}

/// スタック割り当てのテスト
pub fn test_stack_allocation() -> bool {
    // デフォルトサイズ（64KB）でスタックを割り当て
    let stack_ptr = memory::allocate_default_stack();

    if stack_ptr.is_none() {
        return false;
    }

    let sp = stack_ptr.unwrap();

    // スタックポインタは0でないはず
    if sp == 0 {
        return false;
    }

    // スタック使用状況を確認
    let (count, total_size) = memory::get_stack_usage();

    // 1つのスタックが割り当てられているはず
    if count != 1 {
        return false;
    }

    // サイズは64KB以上（ページアライン後）
    if total_size < 64 * 1024 {
        return false;
    }

    // スタックを解放
    let freed = memory::deallocate_stack(sp);

    if !freed {
        return false;
    }

    // 解放後は0になるはず
    let (count_after, _) = memory::get_stack_usage();
    count_after == 0
}

/// カスタムサイズスタック割り当てのテスト
pub fn test_custom_stack_allocation() -> bool {
    // 128KBのスタックを割り当て
    let stack_ptr = memory::allocate_stack(128 * 1024);

    if stack_ptr.is_none() {
        return false;
    }

    let sp = stack_ptr.unwrap();

    // スタック使用状況を確認
    let (count, total_size) = memory::get_stack_usage();

    // サイズは128KB以上（ページアライン後）
    if total_size < 128 * 1024 {
        return false;
    }

    // スタックを解放
    memory::deallocate_stack(sp);

    true
}

/// 複数スタック割り当てのテスト
pub fn test_multiple_stack_allocation() -> bool {
    let mut stack_ptrs = Vec::new();

    // 5つのスタックを割り当て
    for _ in 0..5 {
        if let Some(sp) = memory::allocate_default_stack() {
            stack_ptrs.push(sp);
        } else {
            // 割り当て失敗
            return false;
        }
    }

    // 5つすべて割り当てられたか確認
    if stack_ptrs.len() != 5 {
        return false;
    }

    // スタック使用状況を確認
    let (count, _) = memory::get_stack_usage();
    if count != 5 {
        return false;
    }

    // すべてのスタックを解放
    for sp in stack_ptrs {
        if !memory::deallocate_stack(sp) {
            return false;
        }
    }

    // 解放後は0になるはず
    let (count_after, _) = memory::get_stack_usage();
    count_after == 0
}

/// 全テストの実行
pub fn run_all_tests() -> bool {
    let tests = [
        ("Heap Usage Test", test_heap_usage as fn() -> bool),
        ("System Memory Info Test", test_system_memory_info),
        ("Stack Allocation Test", test_stack_allocation),
        ("Custom Stack Allocation Test", test_custom_stack_allocation),
        (
            "Multiple Stack Allocation Test",
            test_multiple_stack_allocation,
        ),
    ];

    let mut all_passed = true;

    for (name, test_fn) in &tests {
        let result = test_fn();
        if result {
            // simple::println(&format!("✅ {}: PASSED", name));
        } else {
            // simple::println(&format!("❌ {}: FAILED", name));
            all_passed = false;
        }
    }

    all_passed
}
