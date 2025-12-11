# Retron OS API リファレンス

## カーネルAPI

### メモリ管理

#### `memory::init()`
メモリ管理システムを初期化します。

```rust
pub fn init()
```

#### `memory::allocate(size: usize) -> Option<*mut u8>`
指定されたサイズのメモリを割り当てます。

**パラメータ:**
- `size`: 割り当てるメモリサイズ（バイト）

**戻り値:**
- `Some(ptr)`: 割り当てられたメモリのポインタ
- `None`: 割り当て失敗

#### `memory::deallocate(ptr: *mut u8, size: usize)`
指定されたメモリを解放します。

**パラメータ:**
- `ptr`: 解放するメモリのポインタ
- `size`: 解放するメモリサイズ

### タスク管理

#### `task::create_task(priority: TaskPriority, stack_size: usize, entry_point: fn()) -> Option<TaskId>`
新しいタスクを作成します。

**パラメータ:**
- `priority`: タスク優先度
- `stack_size`: スタックサイズ
- `entry_point`: タスクエントリーポイント

**戻り値:**
- `Some(task_id)`: 作成されたタスクID
- `None`: 作成失敗

#### `task::start_task(task_id: TaskId) -> bool`
指定されたタスクを開始します。

**パラメータ:**
- `task_id`: 開始するタスクID

**戻り値:**
- `true`: 開始成功
- `false`: 開始失敗

#### `task::exit_task(task_id: TaskId) -> bool`
指定されたタスクを終了します。

**パラメータ:**
- `task_id`: 終了するタスクID

**戻り値:**
- `true`: 終了成功
- `false`: 終了失敗

### デバイス管理

#### `device::register_device(device_type: DeviceType, name: &str, base_address: usize, interrupt_number: Option<u8>) -> Option<usize>`
新しいデバイスを登録します。

**パラメータ:**
- `device_type`: デバイスタイプ
- `name`: デバイス名
- `base_address`: ベースアドレス
- `interrupt_number`: 割り込み番号

**戻り値:**
- `Some(device_id)`: 登録されたデバイスID
- `None`: 登録失敗

#### `device::initialize_device(device_id: usize) -> bool`
指定されたデバイスを初期化します。

**パラメータ:**
- `device_id`: 初期化するデバイスID

**戻り値:**
- `true`: 初期化成功
- `false`: 初期化失敗

### 割り込み処理

#### `interrupt::register_handler(interrupt_number: u8, handler: InterruptHandler)`
割り込みハンドラーを登録します。

**パラメータ:**
- `interrupt_number`: 割り込み番号
- `handler`: 割り込みハンドラー関数

## Core API

### プラットフォーム検出

#### `platform::detect_platform() -> Platform`
現在のプラットフォームを検出します。

**戻り値:**
- `Platform::Laptop`: Laptopプラットフォーム
- `Platform::Mobile`: Mobileプラットフォーム
- `Platform::Unknown`: 不明なプラットフォーム

#### `platform::get_platform_info() -> PlatformInfo`
プラットフォーム情報を取得します。

**戻り値:**
- `PlatformInfo`: プラットフォーム情報構造体

### デバイス管理

#### `device::enable_device(device_type: &str) -> bool`
指定されたデバイスを有効化します。

**パラメータ:**
- `device_type`: デバイスタイプ文字列

**戻り値:**
- `true`: 有効化成功
- `false`: 有効化失敗

#### `device::disable_device(device_type: &str) -> bool`
指定されたデバイスを無効化します。

**パラメータ:**
- `device_type`: デバイスタイプ文字列

**戻り値:**
- `true`: 無効化成功
- `false`: 無効化失敗

## UI API

### グラフィックス

#### `graphics::init()`
グラフィックスシステムを初期化します。

#### `graphics::get_graphics_system() -> Option<&mut GraphicsSystem>`
グラフィックスシステムを取得します。

**戻り値:**
- `Some(system)`: グラフィックスシステム
- `None`: 取得失敗

#### `graphics::set_color(color: Rgb565)`
現在の描画色を設定します。

**パラメータ:**
- `color`: RGB565色

#### `graphics::draw_rectangle(x: u32, y: u32, width: u32, height: u32)`
矩形を描画します。

**パラメータ:**
- `x`, `y`: 位置
- `width`, `height`: サイズ

#### `graphics::draw_circle(center_x: u32, center_y: u32, radius: u32)`
円を描画します。

**パラメータ:**
- `center_x`, `center_y`: 中心座標
- `radius`: 半径

#### `graphics::draw_text(x: u32, y: u32, text: &str)`
テキストを描画します。

**パラメータ:**
- `x`, `y`: 位置
- `text`: 描画するテキスト

### 入力処理

#### `input::init()`
入力システムを初期化します。

#### `input::get_input_system() -> Option<&mut InputSystem>`
入力システムを取得します。

**戻り値:**
- `Some(system)`: 入力システム
- `None`: 取得失敗

#### `input::handle_key_event(key: KeyCode, pressed: bool, modifiers: KeyModifiers)`
キーイベントを処理します。

**パラメータ:**
- `key`: キーコード
- `pressed`: 押下状態
- `modifiers`: 修飾キー

#### `input::handle_mouse_event(button: MouseButton, pressed: bool, x: i32, y: i32)`
マウスイベントを処理します。

**パラメータ:**
- `button`: マウスボタン
- `pressed`: 押下状態
- `x`, `y`: 座標

### ウィンドウ管理

#### `window::init()`
ウィンドウシステムを初期化します。

#### `window::create_window(title: String, x: i32, y: i32, width: u32, height: u32) -> Option<WindowId>`
新しいウィンドウを作成します。

**パラメータ:**
- `title`: ウィンドウタイトル
- `x`, `y`: 位置
- `width`, `height`: サイズ

**戻り値:**
- `Some(window_id)`: 作成されたウィンドウID
- `None`: 作成失敗

#### `window::focus_window(window_id: WindowId) -> bool`
指定されたウィンドウにフォーカスします。

**パラメータ:**
- `window_id`: フォーカスするウィンドウID

**戻り値:**
- `true`: フォーカス成功
- `false`: フォーカス失敗

### ウィジェット

#### `widget::create_button(x: i32, y: i32, width: u32, height: u32, text: String) -> Option<WidgetId>`
ボタンウィジェットを作成します。

**パラメータ:**
- `x`, `y`: 位置
- `width`, `height`: サイズ
- `text`: ボタンテキスト

**戻り値:**
- `Some(widget_id)`: 作成されたウィジェットID
- `None`: 作成失敗

#### `widget::create_label(x: i32, y: i32, text: String) -> Option<WidgetId>`
ラベルウィジェットを作成します。

**パラメータ:**
- `x`, `y`: 位置
- `text`: ラベルテキスト

**戻り値:**
- `Some(widget_id)`: 作成されたウィジェットID
- `None`: 作成失敗

## Robot API

### アクチュエーター制御

#### `actuator::add_actuator(actuator_type: ActuatorType) -> Option<usize>`
新しいアクチュエーターを追加します。

**パラメータ:**
- `actuator_type`: アクチュエータータイプ

**戻り値:**
- `Some(actuator_id)`: 追加されたアクチュエーターID
- `None`: 追加失敗

#### `actuator::set_actuator_position(id: usize, position: f32) -> bool`
アクチュエーターの位置を設定します。

**パラメータ:**
- `id`: アクチュエーターID
- `position`: 目標位置

**戻り値:**
- `true`: 設定成功
- `false`: 設定失敗

#### `actuator::set_actuator_velocity(id: usize, velocity: f32) -> bool`
アクチュエーターの速度を設定します。

**パラメータ:**
- `id`: アクチュエーターID
- `velocity`: 目標速度

**戻り値:**
- `true`: 設定成功
- `false`: 設定失敗

### センサー制御

#### `sensor::add_sensor(sensor_type: SensorType) -> Option<usize>`
新しいセンサーを追加します。

**パラメータ:**
- `sensor_type`: センサータイプ

**戻り値:**
- `Some(sensor_id)`: 追加されたセンサーID
- `None`: 追加失敗

#### `sensor::set_sensor_value(id: usize, value: SensorValue) -> bool`
センサー値を設定します。

**パラメータ:**
- `id`: センサーID
- `value`: センサー値

**戻り値:**
- `true`: 設定成功
- `false`: 設定失敗

### モーション制御

#### `motion::set_target_position(x: f32, y: f32, z: f32)`
目標位置を設定します。

**パラメータ:**
- `x`, `y`, `z`: 目標座標

#### `motion::set_target_orientation(roll: f32, pitch: f32, yaw: f32)`
目標姿勢を設定します。

**パラメータ:**
- `roll`, `pitch`, `yaw`: 目標姿勢（ラジアン）

#### `motion::get_position() -> Option<Position>`
現在位置を取得します。

**戻り値:**
- `Some(position)`: 現在位置
- `None`: 取得失敗

### ナビゲーション

#### `navigation::set_goal(x: f32, y: f32, z: f32, roll: Option<f32>, pitch: Option<f32>, yaw: Option<f32>)`
ナビゲーション目標を設定します。

**パラメータ:**
- `x`, `y`, `z`: 目標座標
- `roll`, `pitch`, `yaw`: 目標姿勢（オプション）

#### `navigation::start_navigation()`
ナビゲーションを開始します。

#### `navigation::stop_navigation()`
ナビゲーションを停止します。

### 通信

#### `communication::add_channel(protocol: CommunicationProtocol) -> Option<usize>`
通信チャンネルを追加します。

**パラメータ:**
- `protocol`: 通信プロトコル

**戻り値:**
- `Some(channel_id)`: 追加されたチャンネルID
- `None`: 追加失敗

#### `communication::send_message(channel_id: usize, data: Vec<u8>, priority: u8) -> bool`
メッセージを送信します。

**パラメータ:**
- `channel_id`: チャンネルID
- `data`: 送信データ
- `priority`: 優先度

**戻り値:**
- `true`: 送信成功
- `false`: 送信失敗

### 制御システム

#### `control::add_command(command: ControlCommand) -> bool`
制御コマンドを追加します。

**パラメータ:**
- `command`: 制御コマンド

**戻り値:**
- `true`: 追加成功
- `false`: 追加失敗

#### `control::set_control_mode(mode: ControlMode)`
制御モードを設定します。

**パラメータ:**
- `mode`: 制御モード

#### `control::emergency_stop()`
緊急停止を実行します。

## エラー処理

### エラーコード

- `0`: 成功
- `1`: メモリ不足
- `2`: デバイスエラー
- `3`: タスクエラー
- `4`: 通信エラー
- `5`: 制御エラー

### エラーハンドリング

```rust
match result {
    Ok(value) => {
        // 成功処理
    },
    Err(error) => {
        error!("Error: {}", error);
        // エラー処理
    }
}
```


