# クイックスタート: カーネルデバッグ手順

## 前提条件
- QEMUがインストールされていること
- `aarch64-linux-gnu-gdb`がインストールされていること
- カーネルがビルド済みであること

## 手順

### 1. QEMUを起動（GDBサーバー有効）

```bash
cd /home/miyasaka/source/retron
./run-tkernel-qemu.sh
```

QEMUは停止状態で起動します（`-S`オプションのため）。

### 2. 別のターミナルでGDBを起動

```bash
cd /home/miyasaka/source/retron
./debug-kernel.sh
```

または、手動で：

```bash
aarch64-linux-gnu-gdb third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
(gdb) target remote :1234
(gdb) break start
(gdb) break main
(gdb) break uart_pl011_init
(gdb) continue
```

### 3. デバッグコマンド

#### 基本的なコマンド
- `continue` (または `c`): 実行を続ける
- `step` (または `s`): ステップイン
- `next` (または `n`): ステップオーバー
- `info registers`: レジスタの状態を表示
- `x/10i $pc`: 現在の命令を10個表示
- `print variable`: 変数の値を表示

#### メモリ/レジスタの確認
- `x/16x 0x40200000`: 0x40200000から16バイトを16進数で表示
- `x/16x 0x09000000`: UARTレジスタ（PL011）の状態を確認
- `info registers x0 x1 x2`: 特定のレジスタを表示

#### UARTの確認
- `break uart_pl011_putchar`: UART出力関数にブレークポイント
- `print c`: 出力される文字を確認
- `x/4x 0x09000000`: UARTレジスタの状態を確認

### 4. 確認すべきポイント

1. **エントリーポイント**
   - `start`で停止するか確認
   - PCレジスタが`0x40200000`になっているか確認

2. **初期化シーケンス**
   - `main`が呼ばれるか確認
   - `cpu_initialize`が呼ばれるか確認
   - `init_device`が呼ばれるか確認

3. **UART初期化**
   - `uart_pl011_init`が呼ばれるか確認
   - UARTレジスタ（0x09000000）が正しく設定されているか確認

4. **出力**
   - `uart_pl011_putchar`が呼ばれるか確認
   - 出力される文字が正しいか確認

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

## 次のステップ

動作確認ができたら：
1. UART出力が表示されることを確認
2. カーネルの初期化シーケンスが正常に動作することを確認
3. 必要に応じて、残りの高優先度項目（GIC初期化、パラメータコピー）を実装


