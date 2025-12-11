//! ターミナルのテスト機能

use crate::terminal::*;
use core::option::Option::{None, Some};
use core::result::Result::{Err, Ok};

/// ターミナルのテストを実行
pub fn test_terminal() -> bool {
    // ターミナルの初期化テスト
    if init_terminal().is_err() {
        return false;
    }

    // 基本的なコマンドテスト
    if !test_basic_commands() {
        return false;
    }

    // コマンド登録テスト
    if !test_command_registration() {
        return false;
    }

    // 入力処理テスト
    if !test_input_processing() {
        return false;
    }

    // 履歴機能テスト
    if !test_history_functionality() {
        return false;
    }

    true
}

/// 基本的なコマンドのテスト
fn test_basic_commands() -> bool {
    let terminal = match get_terminal() {
        Ok(terminal) => terminal,
        Err(_) => return false,
    };

    // help コマンドのテスト
    match terminal.execute_command("help") {
        Ok(_) => {}
        Err(_) => return false,
    }

    // version コマンドのテスト
    match terminal.execute_command("version") {
        Ok(_) => {}
        Err(_) => return false,
    }

    // info コマンドのテスト
    match terminal.execute_command("info") {
        Ok(_) => {}
        Err(_) => return false,
    }

    // ls コマンドのテスト
    match terminal.execute_command("ls") {
        Ok(_) => {}
        Err(_) => return false,
    }

    // pwd コマンドのテスト
    match terminal.execute_command("pwd") {
        Ok(_) => {}
        Err(_) => return false,
    }

    // echo コマンドのテスト
    match terminal.execute_command("echo Hello, World!") {
        Ok(_) => {}
        Err(_) => return false,
    }

    // clear コマンドのテスト
    match terminal.execute_command("clear") {
        Ok(_) => {}
        Err(_) => return false,
    }

    true
}

/// コマンド登録のテスト
fn test_command_registration() -> bool {
    let terminal = match get_terminal() {
        Ok(terminal) => terminal,
        Err(_) => return false,
    };

    // 新しいコマンドを登録
    match terminal.register_command("test", CommandType::Builtin, "Test command") {
        Ok(_) => {}
        Err(_) => return false,
    }

    // 登録されたコマンドを検索
    match terminal.find_command("test") {
        Some(_) => {}
        None => return false,
    }

    // 存在しないコマンドを検索
    if terminal.find_command("nonexistent").is_some() {
        return false;
    }

    true
}

/// 入力処理のテスト
fn test_input_processing() -> bool {
    let terminal = match get_terminal() {
        Ok(terminal) => terminal,
        Err(_) => return false,
    };

    // 通常の文字入力
    match terminal.process_input(b'h') {
        Ok(_) => {}
        Err(_) => return false,
    }

    match terminal.process_input(b'e') {
        Ok(_) => {}
        Err(_) => return false,
    }

    match terminal.process_input(b'l') {
        Ok(_) => {}
        Err(_) => return false,
    }

    match terminal.process_input(b'l') {
        Ok(_) => {}
        Err(_) => return false,
    }

    match terminal.process_input(b'o') {
        Ok(_) => {}
        Err(_) => return false,
    }

    // バックスペース
    match terminal.process_input(b'\x08') {
        Ok(_) => {}
        Err(_) => return false,
    }

    // エンターキー
    match terminal.process_input(b'\n') {
        Ok(_) => {}
        Err(_) => return false,
    }

    true
}

/// 履歴機能のテスト
fn test_history_functionality() -> bool {
    let terminal = match get_terminal() {
        Ok(terminal) => terminal,
        Err(_) => return false,
    };

    // 複数のコマンドを実行して履歴に追加
    let _ = terminal.execute_command("help");
    let _ = terminal.execute_command("version");
    let _ = terminal.execute_command("info");

    // 履歴の確認
    if terminal.history_count < 3 {
        return false;
    }

    true
}

/// ターミナルの詳細テスト
pub fn test_terminal_detailed() -> bool {
    // ターミナルの初期化
    if init_terminal().is_err() {
        return false;
    }

    let terminal = match get_terminal() {
        Ok(terminal) => terminal,
        Err(_) => return false,
    };

    // 複数のコマンドを実行
    let commands = [
        "help",
        "version",
        "info",
        "ls",
        "pwd",
        "echo Hello, World!",
        "clear",
    ];

    for command in &commands {
        if terminal.execute_command(command).is_err() {
            return false;
        }
    }

    // 履歴の確認
    if terminal.history_count < commands.len() as u8 {
        return false;
    }

    true
}

/// ターミナルのパフォーマンステスト
pub fn test_terminal_performance() -> bool {
    // ターミナルの初期化
    if init_terminal().is_err() {
        return false;
    }

    let terminal = match get_terminal() {
        Ok(terminal) => terminal,
        Err(_) => return false,
    };

    // 大量のコマンド実行テスト
    for i in 0..100 {
        let command = if i % 2 == 0 { "help" } else { "version" };
        if terminal.execute_command(command).is_err() {
            return false;
        }
    }

    // 履歴の確認
    if terminal.history_count < 16 {
        // 最大履歴数
        return false;
    }

    true
}
