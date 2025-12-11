//! stdio対応ターミナル
//! 
//! 標準入出力から入力を受け取り、表示を返すターミナル

use core::option::Option::{self, Some, None};
use core::result::Result::{self, Ok, Err};

/// stdioターミナルのエラー
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum StdioTerminalError {
    InputError,
    OutputError,
    CommandNotFound,
    InvalidInput,
    SystemError,
}

/// stdioターミナルの結果
pub type StdioTerminalResult<T> = Result<T, StdioTerminalError>;

/// stdioターミナルの状態
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum StdioTerminalState {
    Ready,
    Reading,
    Processing,
    Writing,
    Error,
}

/// stdioターミナル
pub struct StdioTerminal {
    pub state: StdioTerminalState,
    pub input_buffer: [u8; 256], // 入力バッファ
    pub input_len: u8,
    pub output_buffer: [u8; 512], // 出力バッファ
    pub output_len: u8,
    pub prompt: [u8; 32], // プロンプト
    pub prompt_len: u8,
    pub history: [[u8; 256]; 16], // 履歴（最大16個）
    pub history_count: u8,
    pub history_index: u8,
}

impl StdioTerminal {
    /// 新しいstdioターミナルを作成
    pub fn new() -> Self {
        Self {
            state: StdioTerminalState::Ready,
            input_buffer: [0; 256],
            input_len: 0,
            output_buffer: [0; 512],
            output_len: 0,
            prompt: {
                let mut prompt = [0u8; 32];
                let prompt_str = b"retron$ ";
                for (i, &byte) in prompt_str.iter().enumerate() {
                    prompt[i] = byte;
                }
                prompt
            },
            prompt_len: 8,
            history: [[0; 256]; 16],
            history_count: 0,
            history_index: 0,
        }
    }

    /// stdioターミナルを初期化
    pub fn init(&mut self) -> StdioTerminalResult<()> {
        self.state = StdioTerminalState::Ready;
        self.clear_buffers();
        self.print_welcome_message();
        self.show_prompt();
        Ok(())
    }

    /// ウェルカムメッセージを表示
    fn print_welcome_message(&mut self) {
        self.print_line("Retron OS stdio Terminal v1.0");
        self.print_line("Type 'help' for available commands.");
        self.print_line("Type 'exit' to quit.");
    }

    /// プロンプトを表示
    fn show_prompt(&mut self) {
        // プロンプトをコピーして借用チェッカーの問題を回避
        let mut prompt_buffer = [0u8; 32];
        let prompt_len = self.prompt_len as usize;
        for i in 0..prompt_len {
            prompt_buffer[i] = self.prompt[i];
        }
        let prompt = core::str::from_utf8(&prompt_buffer[..prompt_len]).unwrap_or("");
        self.print(prompt);
    }

    /// 文字列を出力
    fn print(&mut self, text: &str) {
        // 実際の実装ではVGAバッファやUARTに出力
        // ここでは簡易的な実装
    }

    /// 行を出力
    fn print_line(&mut self, line: &str) {
        self.print(line);
        self.print("\n");
    }

    /// バッファをクリア
    fn clear_buffers(&mut self) {
        self.input_buffer = [0; 256];
        self.input_len = 0;
        self.output_buffer = [0; 512];
        self.output_len = 0;
    }

    /// 入力文字を処理
    pub fn process_input(&mut self, ch: u8) -> StdioTerminalResult<()> {
        match ch {
            b'\n' | b'\r' => {
                // エンターキー
                self.process_command_line();
            },
            b'\x08' | b'\x7f' => {
                // バックスペース
                if self.input_len > 0 {
                    self.input_len -= 1;
                }
            },
            b'\x03' => {
                // Ctrl+C
                self.print_line("^C");
                self.clear_input();
                self.show_prompt();
            },
            b'\x04' => {
                // Ctrl+D
                self.print_line("^D");
                return Err(StdioTerminalError::InputError);
            },
            _ => {
                // 通常の文字
                if self.input_len < 255 {
                    self.input_buffer[self.input_len as usize] = ch;
                    self.input_len += 1;
                }
            }
        }
        Ok(())
    }

    /// コマンドラインを処理
    fn process_command_line(&mut self) {
        if self.input_len > 0 {
            // コマンドラインをコピーして借用チェッカーの問題を回避
            let mut cmd_buffer = [0u8; 256];
            let cmd_len = core::cmp::min(self.input_len as usize, 255);
            for i in 0..cmd_len {
                cmd_buffer[i] = self.input_buffer[i];
            }
            let command_line = core::str::from_utf8(&cmd_buffer[..cmd_len]).unwrap_or("");
            
            // 履歴に追加
            self.add_to_history(command_line);
            
            // コマンドを実行
            let _ = self.execute_command(command_line);
            
            // 入力バッファをクリア
            self.clear_input();
        }
        
        // プロンプトを表示
        self.show_prompt();
    }

    /// 入力バッファをクリア
    fn clear_input(&mut self) {
        self.input_buffer = [0; 256];
        self.input_len = 0;
    }

    /// コマンドを実行
    fn execute_command(&mut self, command_line: &str) -> StdioTerminalResult<()> {
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
            "history" => self.execute_history(),
            "whoami" => self.execute_whoami(),
            "date" => self.execute_date(),
            "uptime" => self.execute_uptime(),
            _ => {
                // 簡易的な文字列結合
                let mut output = [0u8; 128];
                let mut pos = 0;
                let prefix = b"Command not found: ";
                let command_bytes = command_name.as_bytes();
                
                // プレフィックスを追加
                for &byte in prefix {
                    if pos < 127 {
                        output[pos] = byte;
                        pos += 1;
                    }
                }
                
                // コマンド名を追加
                for &byte in command_bytes {
                    if pos < 127 {
                        output[pos] = byte;
                        pos += 1;
                    }
                }
                
                let output_str = core::str::from_utf8(&output[..pos]).unwrap_or("");
                self.print_error(output_str);
                Err(StdioTerminalError::CommandNotFound)
            }
        }
    }

    /// help コマンドの実行
    fn execute_help(&mut self) -> StdioTerminalResult<()> {
        self.print_line("Available commands:");
        self.print_line("  help     - Show this help message");
        self.print_line("  ls       - List directory contents");
        self.print_line("  pwd      - Print working directory");
        self.print_line("  cd       - Change directory");
        self.print_line("  cat      - Display file contents");
        self.print_line("  echo     - Display text");
        self.print_line("  clear    - Clear screen");
        self.print_line("  exit     - Exit terminal");
        self.print_line("  version  - Show system version");
        self.print_line("  info     - Show system information");
        self.print_line("  history  - Show command history");
        self.print_line("  whoami   - Show current user");
        self.print_line("  date     - Show current date");
        self.print_line("  uptime   - Show system uptime");
        Ok(())
    }

    /// ls コマンドの実行
    fn execute_ls(&mut self) -> StdioTerminalResult<()> {
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
    fn execute_pwd(&mut self) -> StdioTerminalResult<()> {
        self.print_line("/");
        Ok(())
    }

    /// cd コマンドの実行
    fn execute_cd(&mut self, args: &[&str]) -> StdioTerminalResult<()> {
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
    fn execute_cat(&mut self, args: &[&str]) -> StdioTerminalResult<()> {
        if args.is_empty() {
            self.print_error("Usage: cat <file>");
            return Err(StdioTerminalError::InvalidInput);
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
    fn execute_echo(&mut self, args: &[&str]) -> StdioTerminalResult<()> {
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
    fn execute_clear(&mut self) -> StdioTerminalResult<()> {
        // 画面をクリア
        self.clear_buffers();
        Ok(())
    }

    /// exit コマンドの実行
    fn execute_exit(&mut self) -> StdioTerminalResult<()> {
        self.print_line("Goodbye!");
        Err(StdioTerminalError::InputError) // 終了を示す
    }

    /// version コマンドの実行
    fn execute_version(&mut self) -> StdioTerminalResult<()> {
        self.print_line("Retron OS v0.1.0");
        self.print_line("μT-Kernel v3.0");
        self.print_line("stdio Terminal v1.0");
        Ok(())
    }

    /// info コマンドの実行
    fn execute_info(&mut self) -> StdioTerminalResult<()> {
        self.print_line("System Information:");
        self.print_line("  OS: Retron OS v0.1.0");
        self.print_line("  Kernel: Rust-based");
        self.print_line("  Memory: 512MB");
        self.print_line("  Filesystem: OK");
        self.print_line("  Terminal: stdio");
        Ok(())
    }

    /// history コマンドの実行
    fn execute_history(&mut self) -> StdioTerminalResult<()> {
        self.print_line("Command history:");
        for i in 0..self.history_count as usize {
            let history_entry = &self.history[i];
            let entry_str = core::str::from_utf8(history_entry).unwrap_or("");
            if !entry_str.is_empty() {
                // 簡易的な文字列結合
                let mut output = [0u8; 128];
                let mut pos = 0;
                let prefix = b"  ";
                let entry_bytes = entry_str.as_bytes();
                
                // プレフィックスを追加
                for &byte in prefix {
                    if pos < 127 {
                        output[pos] = byte;
                        pos += 1;
                    }
                }
                
                // 履歴番号を追加
                let history_num = i + 1;
                if history_num < 10 {
                    output[pos] = b'0' + history_num as u8;
                    pos += 1;
                } else {
                    output[pos] = b'1';
                    pos += 1;
                    output[pos] = b'0' + (history_num - 10) as u8;
                    pos += 1;
                }
                
                // コロンを追加
                output[pos] = b':';
                pos += 1;
                output[pos] = b' ';
                pos += 1;
                
                // エントリを追加
                for &byte in entry_bytes {
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

    /// whoami コマンドの実行
    fn execute_whoami(&mut self) -> StdioTerminalResult<()> {
        self.print_line("root");
        Ok(())
    }

    /// date コマンドの実行
    fn execute_date(&mut self) -> StdioTerminalResult<()> {
        self.print_line("2024-01-01 00:00:00 UTC");
        Ok(())
    }

    /// uptime コマンドの実行
    fn execute_uptime(&mut self) -> StdioTerminalResult<()> {
        self.print_line("System uptime: 0 days, 0 hours, 0 minutes");
        Ok(())
    }

    /// エラーメッセージを表示
    fn print_error(&mut self, message: &str) {
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
}

/// stdioターミナルのグローバルインスタンス
static mut STDIO_TERMINAL: Option<StdioTerminal> = None;

/// stdioターミナルを初期化
pub fn init_stdio_terminal() -> StdioTerminalResult<()> {
    unsafe {
        STDIO_TERMINAL = Some(StdioTerminal::new());
        STDIO_TERMINAL.as_mut().unwrap().init()
    }
}

/// stdioターミナルのインスタンスを取得
pub fn get_stdio_terminal() -> StdioTerminalResult<&'static mut StdioTerminal> {
    unsafe {
        STDIO_TERMINAL.as_mut().ok_or(StdioTerminalError::SystemError)
    }
}

/// stdioターミナルを実行
pub fn run_stdio_terminal() -> StdioTerminalResult<()> {
    if init_stdio_terminal().is_err() {
        return Err(StdioTerminalError::SystemError);
    }

    let terminal = get_stdio_terminal()?;
    
    // デモンストレーションコマンドの実行
    let _ = terminal.execute_command("help");
    let _ = terminal.execute_command("version");
    let _ = terminal.execute_command("info");
    let _ = terminal.execute_command("ls");
    let _ = terminal.execute_command("pwd");
    let _ = terminal.execute_command("echo Hello, stdio Terminal!");
    let _ = terminal.execute_command("whoami");
    let _ = terminal.execute_command("date");
    let _ = terminal.execute_command("uptime");
    let _ = terminal.execute_command("history");
    
    Ok(())
}

/// stdioシェルを実行
pub fn run_stdio_shell() -> StdioTerminalResult<()> {
    if init_stdio_terminal().is_err() {
        return Err(StdioTerminalError::SystemError);
    }

    let terminal = get_stdio_terminal()?;
    
    // シェル用のデモンストレーション
    let _ = terminal.execute_command("echo Starting stdio shell...");
    let _ = terminal.execute_command("info");
    let _ = terminal.execute_command("ls");
    
    Ok(())
}
