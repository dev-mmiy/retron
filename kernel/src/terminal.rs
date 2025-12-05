//! Retron OS ターミナル機能
//!
//! シンプルなターミナルを実装
//! - コマンド入力
//! - コマンド実行
//! - 出力表示
//! - 履歴管理

use core::option::Option::{self, Some, None};
use core::result::Result::{self, Ok, Err};
use spin::Mutex;

/// ターミナルのエラー
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum TerminalError {
    CommandNotFound,
    InvalidCommand,
    BufferFull,
    InvalidArgument,
    PermissionDenied,
    SystemError,
}

/// ターミナルの結果
pub type TerminalResult<T> = Result<T, TerminalError>;

/// コマンドの種類
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum CommandType {
    Builtin,
    External,
    Alias,
}

/// コマンド情報
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Command {
    pub name: [u8; 32], // コマンド名（最大31文字）
    pub name_len: u8,
    pub command_type: CommandType,
    pub description: [u8; 64], // 説明（最大63文字）
    pub description_len: u8,
}

/// ターミナルの状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum TerminalState {
    Ready,
    Input,
    Processing,
    Error,
}

/// ターミナルの設定
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct TerminalConfig {
    pub prompt: [u8; 32], // プロンプト（最大31文字）
    pub prompt_len: u8,
    pub max_history: u8,
    pub auto_complete: bool,
    pub color_support: bool,
}

/// ターミナルのメイン構造
pub struct Terminal {
    pub state: TerminalState,
    pub config: TerminalConfig,
    pub input_buffer: [u8; 256], // 入力バッファ
    pub input_len: u8,
    pub cursor_pos: u8,
    pub history: [[u8; 256]; 16], // 履歴（最大16個）
    pub history_count: u8,
    pub history_index: u8,
    pub commands: [Option<Command>; 32], // 登録されたコマンド（最大32個）
    pub command_count: u8,
}

impl Terminal {
    /// 新しいターミナルを作成
    pub fn new() -> Self {
        let mut terminal = Self {
            state: TerminalState::Ready,
            config: TerminalConfig {
                prompt: {
                    let mut prompt = [0u8; 32];
                    let prompt_str = b"retron$ ";
                    for (i, &byte) in prompt_str.iter().enumerate() {
                        prompt[i] = byte;
                    }
                    prompt
                },
                prompt_len: 8,
                max_history: 16,
                auto_complete: true,
                color_support: true,
            },
            input_buffer: [0; 256],
            input_len: 0,
            cursor_pos: 0,
            history: [[0; 256]; 16],
            history_count: 0,
            history_index: 0,
            commands: [None; 32],
            command_count: 0,
        };
        
        // デフォルトコマンドを登録
        terminal.register_default_commands();
        terminal
    }

    /// デフォルトコマンドを登録
    fn register_default_commands(&mut self) {
        // help コマンド
        let _ = self.register_command("help", CommandType::Builtin, "Show available commands");

        // ls コマンド
        let _ = self.register_command("ls", CommandType::Builtin, "List directory contents");

        // pwd コマンド
        let _ = self.register_command("pwd", CommandType::Builtin, "Print working directory");

        // cd コマンド
        let _ = self.register_command("cd", CommandType::Builtin, "Change directory");

        // cat コマンド
        let _ = self.register_command("cat", CommandType::Builtin, "Display file contents");

        // echo コマンド
        let _ = self.register_command("echo", CommandType::Builtin, "Display text");

        // clear コマンド
        let _ = self.register_command("clear", CommandType::Builtin, "Clear screen");

        // exit コマンド
        let _ = self.register_command("exit", CommandType::Builtin, "Exit terminal");

        // version コマンド
        let _ = self.register_command("version", CommandType::Builtin, "Show system version");

        // info コマンド
        let _ = self.register_command("info", CommandType::Builtin, "Show system information");
    }

    /// コマンドを登録
    pub fn register_command(&mut self, name: &str, command_type: CommandType, description: &str) -> TerminalResult<()> {
        if self.command_count >= 32 {
            return Err(TerminalError::BufferFull);
        }

        let mut command = Command {
            name: [0; 32],
            name_len: 0,
            command_type,
            description: [0; 64],
            description_len: 0,
        };

        // コマンド名を設定
        let name_bytes = name.as_bytes();
        if name_bytes.len() > 31 {
            return Err(TerminalError::InvalidCommand);
        }
        for (i, &byte) in name_bytes.iter().enumerate() {
            command.name[i] = byte;
        }
        command.name_len = name_bytes.len() as u8;

        // 説明を設定
        let desc_bytes = description.as_bytes();
        if desc_bytes.len() > 63 {
            return Err(TerminalError::InvalidCommand);
        }
        for (i, &byte) in desc_bytes.iter().enumerate() {
            command.description[i] = byte;
        }
        command.description_len = desc_bytes.len() as u8;

        // コマンドを登録
        self.commands[self.command_count as usize] = Some(command);
        self.command_count += 1;

        Ok(())
    }

    /// コマンドを検索
    pub fn find_command(&self, name: &str) -> Option<&Command> {
        for i in 0..self.command_count as usize {
            if let Some(command) = &self.commands[i] {
                if self.compare_strings(&command.name, command.name_len as usize, name) {
                    return Some(command);
                }
            }
        }
        None
    }

    /// 文字列を比較
    fn compare_strings(&self, str1: &[u8], len1: usize, str2: &str) -> bool {
        if len1 != str2.len() {
            return false;
        }
        for i in 0..len1 {
            if str1[i] != str2.as_bytes()[i] {
                return false;
            }
        }
        true
    }

    /// コマンドを実行
    pub fn execute_command(&mut self, command_line: &str) -> TerminalResult<()> {
        let parts = command_line.split(' ');
        let mut args: [&str; 8] = [""; 8];
        let mut arg_count = 0;

        for part in parts {
            if !part.is_empty() {
                if arg_count < 8 {
                    args[arg_count] = part;
                    arg_count += 1;
                }
            }
        }

        if arg_count == 0 {
            return Ok(());
        }

        let command_name = args[0];
        match command_name {
            "help" => self.execute_help(),
            "ls" => self.execute_ls(),
            "pwd" => self.execute_pwd(),
            "cd" => self.execute_cd(&args[1..arg_count]),
            "cat" => self.execute_cat(&args[1..arg_count]),
            "echo" => self.execute_echo(&args[1..arg_count]),
            "clear" => self.execute_clear(),
            "exit" => self.execute_exit(),
            "version" => self.execute_version(),
            "info" => self.execute_info(),
            _ => {
                self.print_error("Command not found");
                Err(TerminalError::CommandNotFound)
            }
        }
    }

    /// help コマンドの実行
    fn execute_help(&self) -> TerminalResult<()> {
        self.print_line("Available commands:");
        for i in 0..self.command_count as usize {
            if let Some(command) = &self.commands[i] {
                let name = core::str::from_utf8(&command.name[..command.name_len as usize]).unwrap_or("");
                let desc = core::str::from_utf8(&command.description[..command.description_len as usize]).unwrap_or("");
                // 簡易的な文字列結合（format!マクロの代わり）
                let mut output = [0u8; 128];
                let mut pos = 0;
                let name_bytes = name.as_bytes();
                let desc_bytes = desc.as_bytes();
                
                // "  " を追加
                output[pos] = b' ';
                pos += 1;
                output[pos] = b' ';
                pos += 1;
                
                // コマンド名を追加
                for &byte in name_bytes {
                    if pos < 127 {
                        output[pos] = byte;
                        pos += 1;
                    }
                }
                
                // " - " を追加
                if pos < 125 {
                    output[pos] = b' ';
                    pos += 1;
                    output[pos] = b'-';
                    pos += 1;
                    output[pos] = b' ';
                    pos += 1;
                }
                
                // 説明を追加
                for &byte in desc_bytes {
                    if pos < 127 {
                        output[pos] = byte;
                        pos += 1;
                    }
                }
                
                let output_str = core::str::from_utf8(&output[..pos]).unwrap_or("");
                self.print_line(output_str);
            }
        }
        Ok(())
    }

    /// ls コマンドの実行
    fn execute_ls(&self) -> TerminalResult<()> {
        self.print_line("Directory listing:");
        self.print_line("  .");
        self.print_line("  ..");
        self.print_line("  bin/");
        self.print_line("  etc/");
        self.print_line("  home/");
        self.print_line("  var/");
        self.print_line("  tmp/");
        Ok(())
    }

    /// pwd コマンドの実行
    fn execute_pwd(&self) -> TerminalResult<()> {
        self.print_line("/");
        Ok(())
    }

    /// cd コマンドの実行
    fn execute_cd(&self, args: &[&str]) -> TerminalResult<()> {
        if args.is_empty() {
            self.print_line("/");
        } else {
            let path = args[0];
            // 簡易的な文字列結合
            let mut output = [0u8; 64];
            let mut pos = 0;
            let prefix = b"Changed directory to: ";
            let path_bytes = path.as_bytes();
            
            // プレフィックスを追加
            for &byte in prefix {
                if pos < 63 {
                    output[pos] = byte;
                    pos += 1;
                }
            }
            
            // パスを追加
            for &byte in path_bytes {
                if pos < 63 {
                    output[pos] = byte;
                    pos += 1;
                }
            }
            
            let output_str = core::str::from_utf8(&output[..pos]).unwrap_or("");
            self.print_line(output_str);
        }
        Ok(())
    }

    /// cat コマンドの実行
    fn execute_cat(&self, args: &[&str]) -> TerminalResult<()> {
        if args.is_empty() {
            self.print_error("Usage: cat <file>");
            return Err(TerminalError::InvalidArgument);
        }
        let filename = args[0];
        // 簡易的な文字列結合
        let mut output = [0u8; 64];
        let mut pos = 0;
        let prefix = b"Contents of ";
        let suffix = b":";
        let filename_bytes = filename.as_bytes();
        
        // プレフィックスを追加
        for &byte in prefix {
            if pos < 63 {
                output[pos] = byte;
                pos += 1;
            }
        }
        
        // ファイル名を追加
        for &byte in filename_bytes {
            if pos < 63 {
                output[pos] = byte;
                pos += 1;
            }
        }
        
        // サフィックスを追加
        for &byte in suffix {
            if pos < 63 {
                output[pos] = byte;
                pos += 1;
            }
        }
        
        let output_str = core::str::from_utf8(&output[..pos]).unwrap_or("");
        self.print_line(output_str);
        self.print_line("  (File contents would be displayed here)");
        Ok(())
    }

    /// echo コマンドの実行
    fn execute_echo(&self, args: &[&str]) -> TerminalResult<()> {
        // 簡易的な文字列結合
        let mut output = [0u8; 256];
        let mut pos = 0;
        
        for (i, arg) in args.iter().enumerate() {
            if i > 0 && pos < 255 {
                output[pos] = b' ';
                pos += 1;
            }
            
            for &byte in arg.as_bytes() {
                if pos < 255 {
                    output[pos] = byte;
                    pos += 1;
                }
            }
        }
        
        let output_str = core::str::from_utf8(&output[..pos]).unwrap_or("");
        self.print_line(output_str);
        Ok(())
    }

    /// clear コマンドの実行
    fn execute_clear(&self) -> TerminalResult<()> {
        // VGAバッファをクリア
        self.clear_screen();
        Ok(())
    }

    /// exit コマンドの実行
    fn execute_exit(&self) -> TerminalResult<()> {
        self.print_line("Goodbye!");
        Ok(())
    }

    /// version コマンドの実行
    fn execute_version(&self) -> TerminalResult<()> {
        self.print_line("Retron OS v0.1.0");
        self.print_line("μT-Kernel v3.0");
        self.print_line("Terminal v1.0");
        Ok(())
    }

    /// info コマンドの実行
    fn execute_info(&self) -> TerminalResult<()> {
        self.print_line("System Information:");
        self.print_line("  OS: Retron OS v0.1.0");
        self.print_line("  Kernel: Rust-based");
        self.print_line("  Memory: 512MB");
        self.print_line("  Filesystem: OK");
        self.print_line("  Terminal: OK");
        Ok(())
    }

    /// エラーメッセージを表示
    fn print_error(&self, message: &str) {
        // 簡易的な文字列結合
        let mut output = [0u8; 128];
        let mut pos = 0;
        let prefix = b"Error: ";
        let message_bytes = message.as_bytes();
        
        // プレフィックスを追加
        for &byte in prefix {
            if pos < 127 {
                output[pos] = byte;
                pos += 1;
            }
        }
        
        // メッセージを追加
        for &byte in message_bytes {
            if pos < 127 {
                output[pos] = byte;
                pos += 1;
            }
        }
        
        let output_str = core::str::from_utf8(&output[..pos]).unwrap_or("");
        self.print_line(output_str);
    }

    /// 行を表示
    fn print_line(&self, _line: &str) {
        // VGAバッファに書き込み
        // 実際の実装ではVGAバッファに直接書き込む
    }

    /// 画面をクリア
    fn clear_screen(&self) {
        // VGAバッファをクリア
        // 実際の実装ではVGAバッファをクリア
    }

    /// プロンプトを表示
    pub fn show_prompt(&self) {
        let prompt = core::str::from_utf8(&self.config.prompt[..self.config.prompt_len as usize]).unwrap_or("");
        self.print_line(prompt);
    }

    /// 入力文字を処理
    pub fn process_input(&mut self, ch: u8) -> TerminalResult<()> {
        match ch {
            b'\n' | b'\r' => {
                // エンターキー
                self.process_command_line();
            },
            b'\x08' | b'\x7f' => {
                // バックスペース
                if self.cursor_pos > 0 {
                    self.cursor_pos -= 1;
                    self.input_len -= 1;
                }
            },
            _ => {
                // 通常の文字
                if self.input_len < 255 {
                    self.input_buffer[self.input_len as usize] = ch;
                    self.input_len += 1;
                    self.cursor_pos += 1;
                }
            }
        }
        Ok(())
    }

    /// コマンドラインを処理
    fn process_command_line(&mut self) {
        if self.input_len > 0 {
            // コマンドラインをコピー
            let mut cmd_buffer = [0u8; 256];
            let cmd_len = core::cmp::min(self.input_len as usize, 255);
            for i in 0..cmd_len {
                cmd_buffer[i] = self.input_buffer[i];
            }
            let cmd_str = core::str::from_utf8(&cmd_buffer[..cmd_len]).unwrap_or("");
            
            // 履歴に追加
            self.add_to_history(cmd_str);
            
            // コマンドを実行
            let _ = self.execute_command(cmd_str);
            
            // 入力バッファをクリア
            self.input_buffer = [0; 256];
            self.input_len = 0;
            self.cursor_pos = 0;
        }
        
        // プロンプトを表示
        self.show_prompt();
    }

    /// 履歴に追加
    fn add_to_history(&mut self, command_line: &str) {
        if self.history_count < 16 {
            let history_entry = &mut self.history[self.history_count as usize];
            let bytes = command_line.as_bytes();
            let len = core::cmp::min(bytes.len(), 255);
            for i in 0..len {
                history_entry[i] = bytes[i];
            }
            self.history_count += 1;
        }
    }

    /// ターミナルを初期化
    pub fn init(&mut self) -> TerminalResult<()> {
        self.state = TerminalState::Ready;
        self.clear_screen();
        self.print_line("Retron OS Terminal v1.0");
        self.print_line("Type 'help' for available commands.");
        self.show_prompt();
        Ok(())
    }
}

/// ターミナルのグローバルインスタンス
static TERMINAL: Mutex<Option<Terminal>> = Mutex::new(None);

/// ターミナルを初期化
pub fn init_terminal() -> TerminalResult<()> {
    // デバッグ出力を追加
    crate::simple::println("DEBUG: init_terminal() - START");

    crate::simple::println("DEBUG: Creating Terminal::new()");
    *TERMINAL.lock() = Some(Terminal::new());
    crate::simple::println("DEBUG: Calling Terminal::init()");
    let result = TERMINAL.lock().as_mut().unwrap().init();
    crate::simple::println("DEBUG: Terminal::init() completed");
    result
}

/// ターミナルのインスタンスを取得
pub fn get_terminal() -> TerminalResult<&'static mut Terminal> {
    // Note: This returns a reference that outlives the MutexGuard
    // In a real implementation, this would need proper lifetime management
    unsafe {
        let ptr = TERMINAL.lock().as_mut().ok_or(TerminalError::SystemError)? as *mut Terminal;
        Ok(&mut *ptr)
    }
}

/// ターミナルデモを実行
pub fn terminal_demo() {
    if init_terminal().is_err() {
        return;
    }

    let terminal = match get_terminal() {
        Ok(terminal) => terminal,
        Err(_) => return,
    };

    // デモコマンドの実行
    let _ = terminal.execute_command("help");
    let _ = terminal.execute_command("version");
    let _ = terminal.execute_command("info");
    let _ = terminal.execute_command("ls");
}
