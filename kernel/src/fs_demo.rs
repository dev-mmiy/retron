//! ファイルシステムのデモンストレーション機能

use core::result::Result::{Ok, Err};
use crate::filesystem::*;

/// ファイルシステムのデモンストレーション
pub fn filesystem_demo() {
    // ファイルシステムの初期化
    if init_filesystem().is_err() {
        return;
    }

    // デモンストレーションの実行
    demo_basic_operations();
    demo_directory_structure();
    demo_file_operations();
    demo_statistics();
}

/// 基本的な操作のデモンストレーション
fn demo_basic_operations() {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return,
    };

    // ファイルの作成
    match fs.create_file("/hello.txt", FilePermissions::default()) {
        Ok(_id) => {
            // ファイル作成成功
        },
        Err(_) => return,
    }

    // ディレクトリの作成
    if let Ok(_id) = fs.create_directory("/documents", FilePermissions::default()) {
        // ディレクトリ作成成功
    }
}

/// ディレクトリ構造のデモンストレーション
fn demo_directory_structure() {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return,
    };

    // 複数のディレクトリを作成
    let directories = [
        "/home",
        "/home/user",
        "/home/user/documents",
        "/home/user/pictures",
        "/var",
        "/var/log",
        "/tmp",
    ];

    for dir in &directories {
        match fs.create_directory(dir, FilePermissions::default()) {
            Ok(_) => {
                // ディレクトリ作成成功
            },
            Err(_) => {
                // エラー（既に存在する可能性）
            }
        }
    }
}

/// ファイル操作のデモンストレーション
fn demo_file_operations() {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return,
    };

    // 複数のファイルを作成
    let files = [
        "/home/user/documents/readme.txt",
        "/home/user/documents/notes.txt",
        "/home/user/pictures/photo1.jpg",
        "/home/user/pictures/photo2.jpg",
        "/var/log/system.log",
        "/var/log/error.log",
        "/tmp/temp_file.tmp",
    ];

    for file in &files {
        match fs.create_file(file, FilePermissions::default()) {
            Ok(_) => {
                // ファイル作成成功
            },
            Err(_) => {
                // エラー（親ディレクトリが存在しない可能性）
            }
        }
    }
}

/// 統計情報のデモンストレーション
fn demo_statistics() {
    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return,
    };

    let _stats = fs.get_stats();
    
    // 統計情報の表示（実際の実装ではVGAバッファに書き込む）
    // ここでは簡易的なデモンストレーション
}

/// ファイルシステムの詳細デモンストレーション
pub fn filesystem_detailed_demo() {
    // ファイルシステムの初期化
    if init_filesystem().is_err() {
        return;
    }

    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return,
    };

    // 複雑なディレクトリ構造を作成
    create_complex_directory_structure(fs);
    
    // ファイルの作成
    create_demo_files(fs);
    
    // 統計情報の表示
    display_filesystem_stats(fs);
}

/// 複雑なディレクトリ構造を作成
fn create_complex_directory_structure(fs: &mut FileSystem) {
    let directories = [
        "/",
        "/bin",
        "/etc",
        "/home",
        "/home/user1",
        "/home/user2",
        "/home/user1/documents",
        "/home/user1/pictures",
        "/home/user2/projects",
        "/home/user2/projects/retron",
        "/var",
        "/var/log",
        "/var/cache",
        "/tmp",
        "/usr",
        "/usr/bin",
        "/usr/lib",
        "/usr/include",
    ];

    for dir in &directories {
        if *dir != "/" {
            match fs.create_directory(dir, FilePermissions::default()) {
                Ok(_) => {
                    // ディレクトリ作成成功
                },
                Err(_) => {
                    // エラー（既に存在する可能性）
                }
            }
        }
    }
}

/// デモファイルを作成
fn create_demo_files(fs: &mut FileSystem) {
    let files = [
        "/bin/retron",
        "/bin/retron-shell",
        "/etc/retron.conf",
        "/home/user1/documents/readme.txt",
        "/home/user1/documents/notes.txt",
        "/home/user1/pictures/photo1.jpg",
        "/home/user1/pictures/photo2.jpg",
        "/home/user2/projects/retron/main.rs",
        "/home/user2/projects/retron/Cargo.toml",
        "/var/log/system.log",
        "/var/log/error.log",
        "/var/cache/temp.cache",
        "/tmp/temp_file.tmp",
        "/usr/bin/gcc",
        "/usr/lib/libc.a",
        "/usr/include/stdio.h",
    ];

    for file in &files {
        match fs.create_file(file, FilePermissions::default()) {
            Ok(_) => {
                // ファイル作成成功
            },
            Err(_) => {
                // エラー（親ディレクトリが存在しない可能性）
            }
        }
    }
}

/// ファイルシステム統計情報を表示
fn display_filesystem_stats(fs: &FileSystem) {
    let _stats = fs.get_stats();
    
    // 統計情報の表示（実際の実装ではVGAバッファに書き込む）
    // ここでは簡易的なデモンストレーション
}

/// ファイルシステムのパフォーマンスデモンストレーション
pub fn filesystem_performance_demo() {
    // ファイルシステムの初期化
    if init_filesystem().is_err() {
        return;
    }

    let fs = match get_filesystem() {
        Ok(fs) => fs,
        Err(_) => return,
    };

    // 大量のファイル作成
    for i in 0..1000 {
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
        match fs.create_file(path_str, FilePermissions::default()) {
            Ok(_) => {
                // ファイル作成成功
            },
            Err(_) => {
                // エラー（ディスクフル等）
                break;
            }
        }
    }

    // 統計情報の表示
    let _stats = fs.get_stats();
    
    // 統計情報の表示（実際の実装ではVGAバッファに書き込む）
    // ここでは簡易的なデモンストレーション
}
