//! Retron OS ファイルシステム
//! 
//! シンプルなファイルシステムを実装
//! - ディレクトリ構造
//! - ファイル操作
//! - メタデータ管理

use core::option::Option::{self, Some, None};
use core::result::Result::{self, Ok, Err};

/// ファイルシステムのエラー
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum FSError {
    NotFound,
    AlreadyExists,
    InvalidPath,
    PermissionDenied,
    DiskFull,
    InvalidOperation,
    Corrupted,
}

/// ファイルシステムの結果
pub type FSResult<T> = Result<T, FSError>;

/// ファイルの種類
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum FileType {
    Regular,
    Directory,
    Symlink,
    Device,
}

/// ファイルの権限
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct FilePermissions {
    pub owner_read: bool,
    pub owner_write: bool,
    pub owner_execute: bool,
    pub group_read: bool,
    pub group_write: bool,
    pub group_execute: bool,
    pub other_read: bool,
    pub other_write: bool,
    pub other_execute: bool,
}

impl Default for FilePermissions {
    fn default() -> Self {
        Self {
            owner_read: true,
            owner_write: true,
            owner_execute: false,
            group_read: true,
            group_write: false,
            group_execute: false,
            other_read: true,
            other_write: false,
            other_execute: false,
        }
    }
}

/// ファイルのメタデータ
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct FileMetadata {
    pub size: u64,
    pub file_type: FileType,
    pub permissions: FilePermissions,
    pub created_at: u64,
    pub modified_at: u64,
    pub accessed_at: u64,
}

/// ファイルシステムのノード
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct FSNode {
    pub id: u32,
    pub name: [u8; 256], // ファイル名（固定長）
    pub name_len: u8,
    pub parent_id: u32,
    pub metadata: FileMetadata,
    pub data_blocks: [u32; 16], // データブロックのインデックス
    pub block_count: u8,
}

/// ファイルシステムのディレクトリエントリ
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct DirectoryEntry {
    pub node_id: u32,
    pub name: [u8; 256],
    pub name_len: u8,
}

/// ファイルシステムの統計情報
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct FSStats {
    pub total_blocks: u32,
    pub used_blocks: u32,
    pub free_blocks: u32,
    pub total_nodes: u32,
    pub used_nodes: u32,
    pub free_nodes: u32,
}

/// ファイルシステムのメイン構造
pub struct FileSystem {
    pub nodes: [Option<FSNode>; 1024], // 最大1024個のノード
    pub root_id: u32,
    pub next_node_id: u32,
    pub stats: FSStats,
}

impl FileSystem {
    /// 新しいファイルシステムを作成
    pub fn new() -> Self {
        let mut fs = Self {
            nodes: [None; 1024],
            root_id: 0,
            next_node_id: 1,
            stats: FSStats {
                total_blocks: 1024,
                used_blocks: 0,
                free_blocks: 1024,
                total_nodes: 1024,
                used_nodes: 0,
                free_nodes: 1024,
            },
        };
        
        // ルートディレクトリを作成
        fs.create_root_directory();
        fs
    }

    /// ルートディレクトリを作成
    fn create_root_directory(&mut self) {
        let root_node = FSNode {
            id: 0,
            name: {
                let mut name = [0u8; 256];
                name[0] = b'/';
                name
            },
            name_len: 1,
            parent_id: 0, // ルートは自分自身を親とする
            metadata: FileMetadata {
                size: 0,
                file_type: FileType::Directory,
                permissions: FilePermissions::default(),
                created_at: 0,
                modified_at: 0,
                accessed_at: 0,
            },
            data_blocks: [0; 16],
            block_count: 0,
        };
        
        self.nodes[0] = Some(root_node);
        self.stats.used_nodes = 1;
        self.stats.free_nodes = 1023;
    }

    /// ノードIDからノードを取得
    pub fn get_node(&self, node_id: u32) -> FSResult<&FSNode> {
        if node_id as usize >= self.nodes.len() {
            return Err(FSError::NotFound);
        }
        
        self.nodes[node_id as usize]
            .as_ref()
            .ok_or(FSError::NotFound)
    }

    /// ノードIDからノードを取得（可変参照）
    pub fn get_node_mut(&mut self, node_id: u32) -> FSResult<&mut FSNode> {
        if node_id as usize >= self.nodes.len() {
            return Err(FSError::NotFound);
        }
        
        self.nodes[node_id as usize]
            .as_mut()
            .ok_or(FSError::NotFound)
    }

    /// パスからノードを取得
    pub fn get_node_by_path(&self, path: &str) -> FSResult<&FSNode> {
        if path.is_empty() || path == "/" {
            return self.get_node(self.root_id);
        }

        let components = path.split('/');
        let mut current_id = self.root_id;

        for component in components {
            if component.is_empty() {
                continue;
            }

            current_id = self.find_child_by_name(current_id, component)?;
        }

        self.get_node(current_id)
    }

    /// 親ノードから子ノードを名前で検索
    fn find_child_by_name(&self, parent_id: u32, name: &str) -> FSResult<u32> {
        let parent = self.get_node(parent_id)?;
        
        if parent.metadata.file_type != FileType::Directory {
            return Err(FSError::InvalidPath);
        }

        // ディレクトリエントリを検索
        for i in 0..parent.block_count as usize {
            let block_id = parent.data_blocks[i];
            // 実際の実装では、ブロックからディレクトリエントリを読み取る
            // ここでは簡易実装
        }

        Err(FSError::NotFound)
    }

    /// ファイルを作成
    pub fn create_file(&mut self, path: &str, permissions: FilePermissions) -> FSResult<u32> {
        let (parent_path, filename) = self.split_path(path)?;
        let parent_id = if parent_path.is_empty() {
            self.root_id
        } else {
            self.get_node_by_path(parent_path)?.id
        };

        // ファイル名の長さをチェック
        if filename.len() > 255 {
            return Err(FSError::InvalidPath);
        }

        // 既存のファイルをチェック
        if self.find_child_by_name(parent_id, filename).is_ok() {
            return Err(FSError::AlreadyExists);
        }

        // 新しいノードを作成
        let node_id = self.allocate_node()?;
        let name_bytes = self.create_name_bytes(filename);
        let new_node = FSNode {
            id: node_id,
            name: name_bytes,
            name_len: filename.len() as u8,
            parent_id,
            metadata: FileMetadata {
                size: 0,
                file_type: FileType::Regular,
                permissions,
                created_at: 0, // 実際の実装では現在時刻
                modified_at: 0,
                accessed_at: 0,
            },
            data_blocks: [0; 16],
            block_count: 0,
        };

        self.nodes[node_id as usize] = Some(new_node);
        self.stats.used_nodes += 1;
        self.stats.free_nodes -= 1;

        Ok(node_id)
    }

    /// ディレクトリを作成
    pub fn create_directory(&mut self, path: &str, permissions: FilePermissions) -> FSResult<u32> {
        let (parent_path, dirname) = self.split_path(path)?;
        let parent_id = if parent_path.is_empty() {
            self.root_id
        } else {
            self.get_node_by_path(parent_path)?.id
        };

        // ディレクトリ名の長さをチェック
        if dirname.len() > 255 {
            return Err(FSError::InvalidPath);
        }

        // 既存のディレクトリをチェック
        if self.find_child_by_name(parent_id, dirname).is_ok() {
            return Err(FSError::AlreadyExists);
        }

        // 新しいノードを作成
        let node_id = self.allocate_node()?;
        let name_bytes = self.create_name_bytes(dirname);
        let new_node = FSNode {
            id: node_id,
            name: name_bytes,
            name_len: dirname.len() as u8,
            parent_id,
            metadata: FileMetadata {
                size: 0,
                file_type: FileType::Directory,
                permissions,
                created_at: 0,
                modified_at: 0,
                accessed_at: 0,
            },
            data_blocks: [0; 16],
            block_count: 0,
        };

        self.nodes[node_id as usize] = Some(new_node);
        self.stats.used_nodes += 1;
        self.stats.free_nodes -= 1;

        Ok(node_id)
    }

    /// パスを親ディレクトリとファイル名に分割
    fn split_path<'a>(&self, path: &'a str) -> FSResult<(&'a str, &'a str)> {
        if path.is_empty() {
            return Err(FSError::InvalidPath);
        }

        if let Some(last_slash) = path.rfind('/') {
            let parent = &path[..last_slash];
            let filename = &path[last_slash + 1..];
            Ok((parent, filename))
        } else {
            Ok(("", path))
        }
    }

    /// 新しいノードIDを割り当て
    fn allocate_node(&mut self) -> FSResult<u32> {
        if self.stats.free_nodes == 0 {
            return Err(FSError::DiskFull);
        }

        let node_id = self.next_node_id;
        self.next_node_id += 1;
        Ok(node_id)
    }

    /// ファイル名をバイト配列に変換
    fn create_name_bytes(&self, name: &str) -> [u8; 256] {
        let mut name_bytes = [0u8; 256];
        for (i, &byte) in name.as_bytes().iter().enumerate() {
            if i < 256 {
                name_bytes[i] = byte;
            }
        }
        name_bytes
    }

    /// ファイルシステムの統計情報を取得
    pub fn get_stats(&self) -> &FSStats {
        &self.stats
    }

    /// ファイルシステムの初期化
    pub fn init(&mut self) -> FSResult<()> {
        // ファイルシステムの初期化処理
        self.stats.used_blocks = 1; // ルートディレクトリ用
        self.stats.free_blocks = self.stats.total_blocks - 1;
        Ok(())
    }

    /// ファイルシステムの整合性チェック
    pub fn check_integrity(&self) -> FSResult<()> {
        // 基本的な整合性チェック
        if self.stats.used_nodes > self.stats.total_nodes {
            return Err(FSError::Corrupted);
        }
        
        if self.stats.used_blocks > self.stats.total_blocks {
            return Err(FSError::Corrupted);
        }

        Ok(())
    }
}

/// ファイルシステムのグローバルインスタンス
static mut FILESYSTEM: Option<FileSystem> = None;

/// ファイルシステムを初期化
pub fn init_filesystem() -> FSResult<()> {
    unsafe {
        // デバッグ出力
        crate::simple::serial_println("DEBUG: Creating new FileSystem");
        FILESYSTEM = Some(FileSystem::new());
        crate::simple::serial_println("DEBUG: FileSystem created, calling init()");
        let result = FILESYSTEM.as_mut().unwrap().init();
        crate::simple::serial_println("DEBUG: FileSystem init() completed");
        result
    }
}

/// ファイルシステムのインスタンスを取得
pub fn get_filesystem() -> FSResult<&'static mut FileSystem> {
    unsafe {
        FILESYSTEM.as_mut().ok_or(FSError::Corrupted)
    }
}

/// ファイルシステムの統計情報を取得
pub fn get_filesystem_stats() -> FSResult<FSStats> {
    let fs = get_filesystem()?;
    Ok(*fs.get_stats())
}
