# T-Kernel 動作確認ガイド

## 前提条件

### 必要なツール
- QEMU: `qemu-system-aarch64` (インストール済み ✅)
- GDB: `aarch64-linux-gnu-gdb` (インストールが必要な場合あり)
- カーネル: `kernel-ram.sys` (288KB, ビルド成功 ✅)

### GDBのインストール（必要な場合）
```bash
sudo apt-get install gdb-multiarch
# または
sudo apt-get install gdb-aarch64-linux-gnu
```

## 動作確認手順

### ステップ1: QEMUを起動（GDBサーバー有効）

ターミナル1で実行:
```bash
cd /home/miyasaka/source/retron
./run-tkernel-qemu.sh
```

QEMUは停止状態で起動します（`-S`オプションのため）。

### ステップ2: GDBで接続

ターミナル2で実行:
```bash
cd /home/miyasaka/source/retron
./debug-kernel.sh
```

または、手動で:
```bash
aarch64-linux-gnu-gdb third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
(gdb) target remote :1234
(gdb) break start
(gdb) break main
(gdb) break uart_pl011_init
(gdb) continue
```

### ステップ3: 動作確認ポイント

#### 3.1 エントリーポイントの確認
- `start`で停止するか確認
- PCレジスタが`0x40200000`になっているか確認
```gdb
(gdb) info registers pc
pc             0x40200000       0x40200000
```

#### 3.2 初期化シーケンスの確認
- `main`が呼ばれるか確認
- `cpu_initialize`が呼ばれるか確認
- `init_device`が呼ばれるか確認

#### 3.3 UART初期化の確認
- `uart_pl011_init`が呼ばれるか確認
- UARTレジスタ（0x09000000）の状態を確認
```gdb
(gdb) x/16x 0x09000000
```

#### 3.4 出力の確認
- `uart_pl011_putchar`が呼ばれるか確認
- QEMUのシリアル出力に文字が表示されるか確認

## トラブルシューティング

### QEMUが起動しない
- QEMUのバージョンを確認: `qemu-system-aarch64 --version`
- メモリサイズを調整: `-m 256M` → `-m 128M`

### GDBが接続できない
- QEMUが`-s -S`オプションで起動されているか確認
- ポート1234が使用可能か確認: `netstat -an | grep 1234`

### ブレークポイントで停止しない
- シンボルが正しくロードされているか確認: `info symbol 0x40200000`
- アドレスが正しいか確認: `nm kernel-ram.sys | grep start`

### UARTから出力されない
- UARTレジスタの状態を確認: `x/16x 0x09000000`
- `uart_pl011_init`が呼ばれているか確認
- UARTのCRレジスタ（0x09000030）が有効になっているか確認

## 期待される動作

正常に動作している場合:
1. `start`で停止（エントリーポイント）
2. `main`で停止（カーネルメイン）
3. `uart_pl011_init`で停止（UART初期化）
4. `uart_pl011_putchar`で停止（文字出力）
5. QEMUのシリアル出力に "Hello, World from ReTron OS!" が表示される

## 次のステップ

動作確認ができたら:
1. UART出力が表示されることを確認
2. カーネルの初期化シーケンスが正常に動作することを確認
3. 必要に応じて、残りの実装項目（保護レベルチェック、タスク独立部の処理）を実装
