//! ファイルシステムのテスト機能

use core::result::Result::{Ok, Err};
use crate::filesystem::*;

/// ファイルシステムのテストを実行
pub fn test_filesystem() -> bool {
    // ファイルシステムの初期化テスト
    if init_filesystem().is_err() {
        return false;
    }

    // 基本的なファイル操作テスト
    if !test_basic_file_operations() {
        return false;
    }

    // ディレクトリ操作テスト
    if !test_directory_operations() {
        return false;
    }

    // 統計情報テスト
    if !test_filesystem_stats() {
        return false;
    }

    // 整合性チェックテスト
    if !test_integrity_check() {
        return false;
    }

    true
}

/// 基本的なファイル操作のテスト
fn test_basic_file_operations() -> bool {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return false,
    };

    // ファイルの作成テスト
    let file_id = match fs.create_file("/test.txt", FilePermissions::default()) {
        Ok(id) => id,
        Err(_) => return false,
    };

    // ファイルの取得テスト
    match fs.get_node(file_id) {
        Ok(node) => {
            if node.metadata.file_type != FileType::Regular {
                return false;
            }
        },
        Err(_) => return false,
    }

    // 既存ファイルの作成テスト（エラーが期待される）
    match fs.create_file("/test.txt", FilePermissions::default()) {
        Ok(_) => return false, // エラーが期待される
        Err(FSError::AlreadyExists) => {}, // 期待されるエラー
        Err(_) => return false,
    }

    true
}

/// ディレクトリ操作のテスト
fn test_directory_operations() -> bool {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return false,
    };

    // ディレクトリの作成テスト
    let dir_id = match fs.create_directory("/test_dir", FilePermissions::default()) {
        Ok(id) => id,
        Err(_) => return false,
    };

    // ディレクトリの取得テスト
    match fs.get_node(dir_id) {
        Ok(node) => {
            if node.metadata.file_type != FileType::Directory {
                return false;
            }
        },
        Err(_) => return false,
    }

    // サブディレクトリの作成テスト
    let subdir_id = match fs.create_directory("/test_dir/subdir", FilePermissions::default()) {
        Ok(id) => id,
        Err(_) => return false,
    };

    // サブディレクトリの取得テスト
    match fs.get_node(subdir_id) {
        Ok(node) => {
            if node.metadata.file_type != FileType::Directory {
                return false;
            }
        },
        Err(_) => return false,
    }

    true
}

/// ファイルシステム統計情報のテスト
fn test_filesystem_stats() -> bool {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return false,
    };

    let stats = fs.get_stats();
    
    // 基本的な統計情報のチェック
    if stats.total_nodes == 0 {
        return false;
    }
    
    if stats.used_nodes == 0 {
        return false;
    }
    
    if stats.free_nodes == 0 {
        return false;
    }

    // ノード数の整合性チェック
    if stats.used_nodes + stats.free_nodes != stats.total_nodes {
        return false;
    }

    true
}

/// 整合性チェックのテスト
fn test_integrity_check() -> bool {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return false,
    };

    // 整合性チェックの実行
    fs.check_integrity().is_ok()
}

/// ファイルシステムの詳細テスト
pub fn test_filesystem_detailed() -> bool {
    // ファイルシステムの初期化
    if init_filesystem().is_err() {
        return false;
    }

    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return false,
    };

    // 複数のファイルとディレクトリを作成
    let test_files = [
        "/file1.txt",
        "/file2.txt",
        "/dir1",
        "/dir2",
        "/dir1/subfile1.txt",
        "/dir2/subfile2.txt",
    ];

    for path in &test_files {
        if path.ends_with(".txt") {
            if fs.create_file(path, FilePermissions::default()).is_err() {
                return false;
            }
        } else if fs.create_directory(path, FilePermissions::default()).is_err() {
            return false;
        }
    }

    // 統計情報の確認
    let stats = fs.get_stats();
    if stats.used_nodes < 7 { // ルート + 6つのテストファイル/ディレクトリ
        return false;
    }

    true
}

/// ファイルシステムのパフォーマンステスト
pub fn test_filesystem_performance() -> bool {
    // ファイルシステムの初期化
    if init_filesystem().is_err() {
        return false;
    }

    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return false,
    };

    // 大量のファイル作成テスト
    for i in 0..100 {
        // 簡易的なパス生成（format!マクロの代わり）
        let mut path = [0u8; 32];
        let path_str = b"/perf_test_";
        for (j, &byte) in path_str.iter().enumerate() {
            path[j] = byte;
        }
        // 数値を文字列に変換（簡易版）
        let mut num = i;
        let mut pos = path_str.len();
        if num == 0 {
            path[pos] = b'0';
            pos += 1;
        } else {
            while num > 0 {
                path[pos] = b'0' + (num % 10) as u8;
                num /= 10;
                pos += 1;
            }
        }
        path[pos] = b'.';
        path[pos + 1] = b't';
        path[pos + 2] = b'x';
        path[pos + 3] = b't';
        path[pos + 4] = 0;
        
        // 文字列に変換
        let path_str = core::str::from_utf8(&path[..pos + 4]).unwrap_or("/perf_test_0.txt");
        if fs.create_file(path_str, FilePermissions::default()).is_err() {
            return false;
        }
    }

    // 統計情報の確認
    let stats = fs.get_stats();
    if stats.used_nodes < 101 { // ルート + 100個のテストファイル
        return false;
    }

    true
}
