//! init.configファイルのパーサー
//!
//! カーネル起動時に自動実行されるプログラムを設定

use core::option::Option::{self, Some, None};
use core::result::Result::{self, Ok, Err};
use spin::Mutex;

/// init.configパーサーのエラー
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum InitConfigError {
    FileNotFound,
    ParseError,
    InvalidSection,
    InvalidKey,
    InvalidValue,
    SystemError,
}

/// init.configパーサーの結果
pub type InitConfigResult<T> = Result<T, InitConfigError>;

/// 設定セクション
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ConfigSection {
    System,
    Programs,
    Startup,
    Environment,
    Logging,
}

/// プログラム設定
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ProgramConfig {
    pub name: [u8; 32], // プログラム名（最大31文字）
    pub name_len: u8,
    pub enabled: bool,
    pub priority: u8,
    pub program_type: [u8; 16], // プログラムタイプ（最大15文字）
    pub program_type_len: u8,
    pub description: [u8; 64], // 説明（最大63文字）
    pub description_len: u8,
}

/// システム設定
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct SystemConfig {
    pub default_shell: [u8; 32], // デフォルトシェル（最大31文字）
    pub default_shell_len: u8,
    pub auto_start: bool,
    pub debug_mode: bool,
    pub verbose_output: bool,
}

/// 環境変数
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct EnvironmentVar {
    pub name: [u8; 32], // 変数名（最大31文字）
    pub name_len: u8,
    pub value: [u8; 128], // 値（最大127文字）
    pub value_len: u8,
}

/// ログ設定
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct LogConfig {
    pub log_level: [u8; 16], // ログレベル（最大15文字）
    pub log_level_len: u8,
    pub log_file: [u8; 64], // ログファイル（最大63文字）
    pub log_file_len: u8,
    pub console_output: bool,
}

/// init.configパーサー
pub struct InitConfigParser {
    pub system_config: SystemConfig,
    pub programs: [Option<ProgramConfig>; 16], // 最大16個のプログラム
    pub program_count: u8,
    pub startup_sequence: [Option<[u8; 32]>; 8], // 最大8個の起動シーケンス
    pub startup_count: u8,
    pub environment_vars: [Option<EnvironmentVar>; 8], // 最大8個の環境変数
    pub env_count: u8,
    pub log_config: LogConfig,
}

impl Default for InitConfigParser {
    fn default() -> Self {
        Self::new()
    }
}

impl InitConfigParser {
    /// 新しいinit.configパーサーを作成
    pub fn new() -> Self {
        Self {
            system_config: SystemConfig {
                default_shell: {
                    let mut shell = [0u8; 32];
                    let shell_str = b"terminal";
                    for (i, &byte) in shell_str.iter().enumerate() {
                        shell[i] = byte;
                    }
                    shell
                },
                default_shell_len: 8,
                auto_start: true,
                debug_mode: false,
                verbose_output: true,
            },
            programs: [None; 16],
            program_count: 0,
            startup_sequence: [None; 8],
            startup_count: 0,
            environment_vars: [None; 8],
            env_count: 0,
            log_config: LogConfig {
                log_level: {
                    let mut level = [0u8; 16];
                    let level_str = b"info";
                    for (i, &byte) in level_str.iter().enumerate() {
                        level[i] = byte;
                    }
                    level
                },
                log_level_len: 4,
                log_file: {
                    let mut file = [0u8; 64];
                    let file_str = b"/var/log/retron.log";
                    for (i, &byte) in file_str.iter().enumerate() {
                        file[i] = byte;
                    }
                    file
                },
                log_file_len: 19,
                console_output: true,
            },
        }
    }

    /// init.configファイルをパース
    pub fn parse_config(&mut self, config_content: &str) -> InitConfigResult<()> {
        let lines = config_content.split('\n');
        let mut current_section = None;
        
        for line in lines {
            let line = line.trim();
            
            // 空行やコメント行をスキップ
            if line.is_empty() || line.starts_with('#') {
                continue;
            }
            
            // セクションの開始
            if line.starts_with('[') && line.ends_with(']') {
                current_section = self.parse_section(line);
                continue;
            }
            
            // キー=値の解析
            if let Some(equal_pos) = line.find('=') {
                let key = line[..equal_pos].trim();
                let value = line[equal_pos + 1..].trim();
                
                match current_section {
                    Some(ConfigSection::System) => {
                        self.parse_system_config(key, value)?;
                    },
                    Some(ConfigSection::Programs) => {
                        self.parse_program_config(key, value)?;
                    },
                    Some(ConfigSection::Startup) => {
                        self.parse_startup_config(key, value)?;
                    },
                    Some(ConfigSection::Environment) => {
                        self.parse_environment_config(key, value)?;
                    },
                    Some(ConfigSection::Logging) => {
                        self.parse_logging_config(key, value)?;
                    },
                    None => return Err(InitConfigError::InvalidSection),
                }
            }
        }
        
        Ok(())
    }

    /// セクションをパース
    fn parse_section(&self, line: &str) -> Option<ConfigSection> {
        let section = line.trim_start_matches('[').trim_end_matches(']');
        match section {
            "system" => Some(ConfigSection::System),
            "programs" => Some(ConfigSection::Programs),
            "startup" => Some(ConfigSection::Startup),
            "environment" => Some(ConfigSection::Environment),
            "logging" => Some(ConfigSection::Logging),
            _ => None,
        }
    }

    /// システム設定をパース
    fn parse_system_config(&mut self, key: &str, value: &str) -> InitConfigResult<()> {
        match key {
            "default_shell" => {
                let value_bytes = value.as_bytes();
                if value_bytes.len() > 31 {
                    return Err(InitConfigError::InvalidValue);
                }
                for (i, &byte) in value_bytes.iter().enumerate() {
                    self.system_config.default_shell[i] = byte;
                }
                self.system_config.default_shell_len = value_bytes.len() as u8;
            },
            "auto_start" => {
                self.system_config.auto_start = value == "true";
            },
            "debug_mode" => {
                self.system_config.debug_mode = value == "true";
            },
            "verbose_output" => {
                self.system_config.verbose_output = value == "true";
            },
            _ => return Err(InitConfigError::InvalidKey),
        }
        Ok(())
    }

    /// プログラム設定をパース
    fn parse_program_config(&mut self, key: &str, value: &str) -> InitConfigResult<()> {
        if self.program_count >= 16 {
            return Err(InitConfigError::SystemError);
        }

        // カンマ区切りの値を手動で分割
        let mut parts = [""; 4];
        let mut part_count = 0;
        let _current_part = 0;
        let mut start = 0;
        
        for (i, &byte) in value.as_bytes().iter().enumerate() {
            if byte == b',' {
                if part_count < 4 {
                    parts[part_count] = &value[start..i];
                    part_count += 1;
                }
                start = i + 1;
            }
        }
        
        // 最後の部分を追加
        if part_count < 4 {
            parts[part_count] = &value[start..];
            part_count += 1;
        }
        
        if part_count != 4 {
            return Err(InitConfigError::ParseError);
        }

        let enabled = parts[0].trim() == "true";
        let priority = parts[1].trim().parse::<u8>().map_err(|_| InitConfigError::InvalidValue)?;
        let program_type = parts[2].trim();
        let description = parts[3].trim();

        let mut program = ProgramConfig {
            name: [0; 32],
            name_len: 0,
            enabled,
            priority,
            program_type: [0; 16],
            program_type_len: 0,
            description: [0; 64],
            description_len: 0,
        };

        // プログラム名を設定
        let key_bytes = key.as_bytes();
        if key_bytes.len() > 31 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in key_bytes.iter().enumerate() {
            program.name[i] = byte;
        }
        program.name_len = key_bytes.len() as u8;

        // プログラムタイプを設定
        let type_bytes = program_type.as_bytes();
        if type_bytes.len() > 15 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in type_bytes.iter().enumerate() {
            program.program_type[i] = byte;
        }
        program.program_type_len = type_bytes.len() as u8;

        // 説明を設定
        let desc_bytes = description.as_bytes();
        if desc_bytes.len() > 63 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in desc_bytes.iter().enumerate() {
            program.description[i] = byte;
        }
        program.description_len = desc_bytes.len() as u8;

        self.programs[self.program_count as usize] = Some(program);
        self.program_count += 1;

        Ok(())
    }

    /// 起動設定をパース
    fn parse_startup_config(&mut self, _key: &str, value: &str) -> InitConfigResult<()> {
        if self.startup_count >= 8 {
            return Err(InitConfigError::SystemError);
        }

        let value_bytes = value.as_bytes();
        if value_bytes.len() > 31 {
            return Err(InitConfigError::InvalidValue);
        }

        let mut startup_name = [0u8; 32];
        for (i, &byte) in value_bytes.iter().enumerate() {
            startup_name[i] = byte;
        }

        self.startup_sequence[self.startup_count as usize] = Some(startup_name);
        self.startup_count += 1;

        Ok(())
    }

    /// 環境変数設定をパース
    fn parse_environment_config(&mut self, key: &str, value: &str) -> InitConfigResult<()> {
        if self.env_count >= 8 {
            return Err(InitConfigError::SystemError);
        }

        let mut env_var = EnvironmentVar {
            name: [0; 32],
            name_len: 0,
            value: [0; 128],
            value_len: 0,
        };

        // 変数名を設定
        let key_bytes = key.as_bytes();
        if key_bytes.len() > 31 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in key_bytes.iter().enumerate() {
            env_var.name[i] = byte;
        }
        env_var.name_len = key_bytes.len() as u8;

        // 値を設定
        let value_bytes = value.as_bytes();
        if value_bytes.len() > 127 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in value_bytes.iter().enumerate() {
            env_var.value[i] = byte;
        }
        env_var.value_len = value_bytes.len() as u8;

        self.environment_vars[self.env_count as usize] = Some(env_var);
        self.env_count += 1;

        Ok(())
    }

    /// ログ設定をパース
    fn parse_logging_config(&mut self, key: &str, value: &str) -> InitConfigResult<()> {
        match key {
            "log_level" => {
                let value_bytes = value.as_bytes();
                if value_bytes.len() > 15 {
                    return Err(InitConfigError::InvalidValue);
                }
                for (i, &byte) in value_bytes.iter().enumerate() {
                    self.log_config.log_level[i] = byte;
                }
                self.log_config.log_level_len = value_bytes.len() as u8;
            },
            "log_file" => {
                let value_bytes = value.as_bytes();
                if value_bytes.len() > 63 {
                    return Err(InitConfigError::InvalidValue);
                }
                for (i, &byte) in value_bytes.iter().enumerate() {
                    self.log_config.log_file[i] = byte;
                }
                self.log_config.log_file_len = value_bytes.len() as u8;
            },
            "console_output" => {
                self.log_config.console_output = value == "true";
            },
            _ => return Err(InitConfigError::InvalidKey),
        }
        Ok(())
    }

    /// 起動シーケンスを取得
    pub fn get_startup_sequence(&self) -> &[Option<[u8; 32]>] {
        &self.startup_sequence[..self.startup_count as usize]
    }

    /// 有効なプログラムを取得
    pub fn get_enabled_programs(&self) -> [Option<&ProgramConfig>; 16] {
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

    /// 環境変数を取得
    pub fn get_environment_vars(&self) -> &[Option<EnvironmentVar>] {
        &self.environment_vars[..self.env_count as usize]
    }

    /// デフォルト設定で初期化
    pub fn init_default(&mut self) -> InitConfigResult<()> {
        // デフォルトプログラムを登録
        self.add_default_program("terminal", true, 100, "terminal", "Interactive terminal")?;
        self.add_default_program("shell", false, 90, "shell", "Command shell")?;
        self.add_default_program("sysinfo", false, 80, "application", "System information")?;
        self.add_default_program("filemanager", false, 70, "application", "File manager")?;
        self.add_default_program("test", false, 60, "application", "System test")?;

        // デフォルト起動シーケンス
        self.add_startup_program("terminal")?;

        // デフォルト環境変数
        self.add_environment_var("PATH", "/bin:/usr/bin:/usr/local/bin")?;
        self.add_environment_var("HOME", "/home")?;
        self.add_environment_var("USER", "root")?;
        self.add_environment_var("SHELL", "/bin/terminal")?;

        Ok(())
    }

    /// デフォルトプログラムを追加
    fn add_default_program(&mut self, name: &str, enabled: bool, priority: u8, program_type: &str, description: &str) -> InitConfigResult<()> {
        if self.program_count >= 16 {
            return Err(InitConfigError::SystemError);
        }

        let mut program = ProgramConfig {
            name: [0; 32],
            name_len: 0,
            enabled,
            priority,
            program_type: [0; 16],
            program_type_len: 0,
            description: [0; 64],
            description_len: 0,
        };

        // プログラム名を設定
        let name_bytes = name.as_bytes();
        if name_bytes.len() > 31 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in name_bytes.iter().enumerate() {
            program.name[i] = byte;
        }
        program.name_len = name_bytes.len() as u8;

        // プログラムタイプを設定
        let type_bytes = program_type.as_bytes();
        if type_bytes.len() > 15 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in type_bytes.iter().enumerate() {
            program.program_type[i] = byte;
        }
        program.program_type_len = type_bytes.len() as u8;

        // 説明を設定
        let desc_bytes = description.as_bytes();
        if desc_bytes.len() > 63 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in desc_bytes.iter().enumerate() {
            program.description[i] = byte;
        }
        program.description_len = desc_bytes.len() as u8;

        self.programs[self.program_count as usize] = Some(program);
        self.program_count += 1;

        Ok(())
    }

    /// 起動プログラムを追加
    fn add_startup_program(&mut self, name: &str) -> InitConfigResult<()> {
        if self.startup_count >= 8 {
            return Err(InitConfigError::SystemError);
        }

        let name_bytes = name.as_bytes();
        if name_bytes.len() > 31 {
            return Err(InitConfigError::InvalidValue);
        }

        let mut startup_name = [0u8; 32];
        for (i, &byte) in name_bytes.iter().enumerate() {
            startup_name[i] = byte;
        }

        self.startup_sequence[self.startup_count as usize] = Some(startup_name);
        self.startup_count += 1;

        Ok(())
    }

    /// 環境変数を追加
    fn add_environment_var(&mut self, name: &str, value: &str) -> InitConfigResult<()> {
        if self.env_count >= 8 {
            return Err(InitConfigError::SystemError);
        }

        let mut env_var = EnvironmentVar {
            name: [0; 32],
            name_len: 0,
            value: [0; 128],
            value_len: 0,
        };

        // 変数名を設定
        let name_bytes = name.as_bytes();
        if name_bytes.len() > 31 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in name_bytes.iter().enumerate() {
            env_var.name[i] = byte;
        }
        env_var.name_len = name_bytes.len() as u8;

        // 値を設定
        let value_bytes = value.as_bytes();
        if value_bytes.len() > 127 {
            return Err(InitConfigError::InvalidValue);
        }
        for (i, &byte) in value_bytes.iter().enumerate() {
            env_var.value[i] = byte;
        }
        env_var.value_len = value_bytes.len() as u8;

        self.environment_vars[self.env_count as usize] = Some(env_var);
        self.env_count += 1;

        Ok(())
    }
}

/// init.configパーサーのグローバルインスタンス
static INIT_CONFIG_PARSER: Mutex<Option<InitConfigParser>> = Mutex::new(None);

/// init.configパーサーを初期化
pub fn init_config_parser() -> InitConfigResult<()> {
    // デバッグ出力を追加
    crate::simple::println("DEBUG: init_config_parser() - START");

    crate::simple::println("DEBUG: Creating InitConfigParser::new()");
    *INIT_CONFIG_PARSER.lock() = Some(InitConfigParser::new());
    crate::simple::println("DEBUG: Calling init_default()");
    let result = INIT_CONFIG_PARSER.lock().as_mut().unwrap().init_default();
    crate::simple::println("DEBUG: init_default() completed");
    result
}

/// init.configパーサーのインスタンスを取得
pub fn get_config_parser() -> InitConfigResult<&'static mut InitConfigParser> {
    // Note: This returns a reference that outlives the MutexGuard
    // In a real implementation, this would need proper lifetime management
    unsafe {
        let ptr = INIT_CONFIG_PARSER.lock().as_mut().ok_or(InitConfigError::SystemError)? as *mut InitConfigParser;
        Ok(&mut *ptr)
    }
}

/// init.configファイルを読み込み
pub fn load_init_config(config_content: &str) -> InitConfigResult<()> {
    let parser = get_config_parser()?;
    parser.parse_config(config_content)
}

/// ファイルシステムからinit.configを読み込み
pub fn load_init_config_from_filesystem() -> InitConfigResult<()> {
    // デバッグ出力を追加
    crate::simple::println("DEBUG: load_init_config_from_filesystem() - START");
    
    // ファイルシステムからinit.configを読み込む
    // 実際の実装では、ファイルシステムのread_file機能を使用
    crate::simple::println("DEBUG: Using default config content");
    let default_config = r#"
# Retron OS init.config
[system]
default_shell = "terminal"
auto_start = true
debug_mode = false
verbose_output = true

[programs]
terminal = true, 100, terminal, "Interactive terminal"
shell = false, 90, shell, "Command shell"
sysinfo = false, 80, application, "System information"
filemanager = false, 70, application, "File manager"
test = false, 60, application, "System test"

[startup]
1 = "terminal"

[environment]
PATH = "/bin:/usr/bin:/usr/local/bin"
HOME = "/home"
USER = "root"
SHELL = "/bin/terminal"

[logging]
log_level = "info"
log_file = "/var/log/retron.log"
console_output = true
"#;
    
    crate::simple::println("DEBUG: Calling load_init_config()");
    let result = load_init_config(default_config);
    crate::simple::println("DEBUG: load_init_config() completed");
    result
}

/// 起動シーケンスを実行
pub fn execute_startup_sequence() -> InitConfigResult<()> {
    let parser = get_config_parser()?;
    let startup_sequence = parser.get_startup_sequence();
    
    for program_name in startup_sequence.iter().flatten() {
        let program_str = core::str::from_utf8(program_name).unwrap_or("");
        execute_program(program_str)?;
    }
    
    Ok(())
}

/// プログラムを実行
fn execute_program(program_name: &str) -> InitConfigResult<()> {
    match program_name {
        "terminal" => {
            match crate::stdio_terminal::run_stdio_terminal() {
                Ok(_) => Ok(()),
                Err(_) => Err(InitConfigError::SystemError)
            }
        },
        "shell" => {
            match crate::stdio_terminal::run_stdio_shell() {
                Ok(_) => Ok(()),
                Err(_) => Err(InitConfigError::SystemError)
            }
        },
        _ => Err(InitConfigError::InvalidValue)
    }
}
