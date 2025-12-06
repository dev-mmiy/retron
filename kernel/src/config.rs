//! Retron OS 設定システム
//!
//! カーネル起動時に実行するプログラムを設定可能

use core::option::Option::{self, Some, None};
use core::result::Result::{self, Ok, Err};
use spin::Mutex;

/// 設定エラー
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ConfigError {
    InvalidConfig,
    ProgramNotFound,
    ParseError,
    SystemError,
}

/// 設定結果
pub type ConfigResult<T> = Result<T, ConfigError>;

/// プログラムの種類
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ProgramType {
    Terminal,
    Shell,
    Application,
    Service,
    Daemon,
}

/// プログラム情報
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Program {
    pub name: [u8; 32], // プログラム名（最大31文字）
    pub name_len: u8,
    pub program_type: ProgramType,
    pub enabled: bool,
    pub priority: u8, // 優先度（0-255）
    pub description: [u8; 64], // 説明（最大63文字）
    pub description_len: u8,
}

/// システム設定
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct SystemConfig {
    pub default_program: [u8; 32], // デフォルトプログラム名
    pub default_program_len: u8,
    pub auto_start_terminal: bool,
    pub auto_start_shell: bool,
    pub debug_mode: bool,
    pub verbose_output: bool,
    pub max_programs: u8,
}

/// 設定マネージャー
pub struct ConfigManager {
    pub config: SystemConfig,
    pub programs: [Option<Program>; 16], // 登録されたプログラム（最大16個）
    pub program_count: u8,
}

impl Default for ConfigManager {
    fn default() -> Self {
        Self::new()
    }
}

impl ConfigManager {
    /// 新しい設定マネージャーを作成
    pub fn new() -> Self {
        let mut manager = Self {
            config: SystemConfig {
                default_program: {
                    let mut name = [0u8; 32];
                    let name_str = b"terminal";
                    for (i, &byte) in name_str.iter().enumerate() {
                        name[i] = byte;
                    }
                    name
                },
                default_program_len: 8,
                auto_start_terminal: true,
                auto_start_shell: false,
                debug_mode: false,
                verbose_output: true,
                max_programs: 16,
            },
            programs: [None; 16],
            program_count: 0,
        };

        // デフォルトプログラムを登録
        manager.register_default_programs();
        manager
    }

    /// デフォルトプログラムを登録
    fn register_default_programs(&mut self) {
        // ターミナルプログラム
        let _ = self.register_program("terminal", ProgramType::Terminal, true, 100, "Interactive terminal");

        // シェルプログラム
        let _ = self.register_program("shell", ProgramType::Shell, false, 90, "Command shell");

        // システム情報プログラム
        let _ = self.register_program("sysinfo", ProgramType::Application, false, 80, "System information");

        // ファイルマネージャープログラム
        let _ = self.register_program("filemanager", ProgramType::Application, false, 70, "File manager");

        // テストプログラム
        let _ = self.register_program("test", ProgramType::Application, false, 60, "System test");
    }

    /// プログラムを登録
    pub fn register_program(&mut self, name: &str, program_type: ProgramType, enabled: bool, priority: u8, description: &str) -> ConfigResult<()> {
        if self.program_count >= 16 {
            return Err(ConfigError::SystemError);
        }

        let mut program = Program {
            name: [0; 32],
            name_len: 0,
            program_type,
            enabled,
            priority,
            description: [0; 64],
            description_len: 0,
        };

        // プログラム名を設定
        let name_bytes = name.as_bytes();
        if name_bytes.len() > 31 {
            return Err(ConfigError::InvalidConfig);
        }
        for (i, &byte) in name_bytes.iter().enumerate() {
            program.name[i] = byte;
        }
        program.name_len = name_bytes.len() as u8;

        // 説明を設定
        let desc_bytes = description.as_bytes();
        if desc_bytes.len() > 63 {
            return Err(ConfigError::InvalidConfig);
        }
        for (i, &byte) in desc_bytes.iter().enumerate() {
            program.description[i] = byte;
        }
        program.description_len = desc_bytes.len() as u8;

        // プログラムを登録
        self.programs[self.program_count as usize] = Some(program);
        self.program_count += 1;

        Ok(())
    }

    /// プログラムを検索
    pub fn find_program(&self, name: &str) -> Option<&Program> {
        for i in 0..self.program_count as usize {
            if let Some(program) = &self.programs[i] {
                if self.compare_strings(&program.name, program.name_len as usize, name) {
                    return Some(program);
                }
            }
        }
        None
    }

    /// 有効なプログラムを取得
    pub fn get_enabled_programs(&self) -> [Option<&Program>; 16] {
        let mut enabled = [None; 16];
        let mut count = 0;
        for i in 0..self.program_count as usize {
            if let Some(program) = &self.programs[i] {
                if program.enabled && count < 16 {
                    enabled[count] = Some(program);
                    count += 1;
                }
            }
        }
        enabled
    }

    /// デフォルトプログラムを取得
    pub fn get_default_program(&self) -> Option<&Program> {
        let default_name = core::str::from_utf8(&self.config.default_program[..self.config.default_program_len as usize]).unwrap_or("");
        self.find_program(default_name)
    }

    /// 文字列を比較
    fn compare_strings(&self, str1: &[u8], len1: usize, str2: &str) -> bool {
        if len1 != str2.len() {
            return false;
        }
        str1.iter().take(len1).zip(str2.as_bytes()).all(|(a, b)| a == b)
    }

    /// 設定を初期化
    pub fn init(&mut self) -> ConfigResult<()> {
        // 設定の検証
        if self.config.max_programs > 16 {
            return Err(ConfigError::InvalidConfig);
        }

        // デフォルトプログラムの確認
        if self.get_default_program().is_none() {
            return Err(ConfigError::ProgramNotFound);
        }

        Ok(())
    }

    /// 設定情報を表示
    pub fn display_config(&self) {
        // 設定情報の表示（実際の実装ではVGAバッファに書き込む）
    }
}

/// 設定マネージャーのグローバルインスタンス
static CONFIG_MANAGER: Mutex<Option<ConfigManager>> = Mutex::new(None);

/// 設定マネージャーを初期化
pub fn init_config() -> ConfigResult<()> {
    // デバッグ出力を追加
    crate::simple::println("DEBUG: init_config() - START");

    crate::simple::println("DEBUG: Creating ConfigManager::new()");
    *CONFIG_MANAGER.lock() = Some(ConfigManager::new());
    crate::simple::println("DEBUG: Calling ConfigManager::init()");
    let result = CONFIG_MANAGER.lock().as_mut().unwrap().init();
    crate::simple::println("DEBUG: ConfigManager::init() completed");
    result
}

/// 設定マネージャーのインスタンスを取得
pub fn get_config() -> ConfigResult<&'static mut ConfigManager> {
    // Note: This returns a reference that outlives the MutexGuard
    // In a real implementation, this would need proper lifetime management
    unsafe {
        let ptr = CONFIG_MANAGER.lock().as_mut().ok_or(ConfigError::SystemError)? as *mut ConfigManager;
        Ok(&mut *ptr)
    }
}

/// デフォルトプログラムを実行
pub fn run_default_program() -> ConfigResult<()> {
    let config = get_config()?;
    let default_program = config.get_default_program().ok_or(ConfigError::ProgramNotFound)?;
    
        match default_program.name[..default_program.name_len as usize] {
            [b't', b'e', b'r', b'm', b'i', b'n', b'a', b'l'] => {
                // ターミナルプログラムを実行
                match crate::stdio_terminal::run_stdio_terminal() {
                    Ok(_) => Ok(()),
                    Err(_) => Err(ConfigError::SystemError)
                }
            },
            [b's', b'h', b'e', b'l', b'l'] => {
                // シェルプログラムを実行
                match crate::stdio_terminal::run_stdio_shell() {
                    Ok(_) => Ok(()),
                    Err(_) => Err(ConfigError::SystemError)
                }
            },
            _ => Err(ConfigError::ProgramNotFound)
        }
}
