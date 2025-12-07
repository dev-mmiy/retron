//! シンプルなテストプログラム

/// テスト関数
pub fn test_basic_functionality() -> bool {
    // 基本的な算術演算
    let a = 10;
    let b = 20;
    let result = a + b;

    // 結果が期待値と一致するかチェック
    result == 30
}

/// メモリテスト
pub fn test_memory_access() -> bool {
    // スタック上の変数
    let mut stack_var = 42;
    stack_var += 1;

    // 結果が期待値と一致するかチェック
    stack_var == 43
}

/// ループテスト
pub fn test_loop() -> bool {
    let mut counter = 0;

    // 10回ループ
    for _ in 0..10 {
        counter += 1;
    }

    // 結果が期待値と一致するかチェック
    counter == 10
}

/// 全テストの実行
pub fn run_all_tests() -> bool {
    test_basic_functionality() && test_memory_access() && test_loop()
}
