# 完全な動作確認手順

## 前提条件

### 必要なツール
- ✅ QEMU: `qemu-system-aarch64` (インストール済み)
- ⚠️ GDB: `aarch64-linux-gnu-gdb` または `gdb-multiarch` (インストールが必要な場合あり)
- ✅ カーネル: `kernel-ram.sys` (288KB, ビルド成功)

### GDBのインストール（必要な場合）
```bash
sudo apt-get install gdb-multiarch
# または
sudo apt-get install gdb-aarch64-linux-gnu
```

## 動作確認手順

### ステップ1: QEMUを起動（GDBサーバー有効）

**ターミナル1**で実行:
```bash
cd /home/miyasaka/source/retron
./run-tkernel-qemu.sh
```

QEMUは停止状態（`-S`オプション）で起動します。以下のようなメッセージが表示されます：
```
QEMU 4.2.1 monitor - type 'help' for more information
(qemu)
```

### ステップ2: GDBで接続

**ターミナル2**で実行:
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

### ステップ3: 確認ポイント

#### 3.1 エントリーポイントの確認
```gdb
(gdb) info registers pc
pc             0x40200000       0x40200000
```

#### 3.2 初期化シーケンスの確認
- `start`で停止するか確認
- `main`が呼ばれるか確認
- `init_device`が呼ばれるか確認
- `uart_pl011_init`が呼ばれるか確認

#### 3.3 UART初期化の確認
```gdb
(gdb) x/16x 0x09000000
```

UARTレジスタの状態を確認します。

#### 3.4 出力の確認
- `uart_pl011_putchar`が呼ばれるか確認
- QEMUのシリアル出力に文字が表示されるか確認

## 期待される動作

正常に動作している場合：
1. `start`で停止（エントリーポイント）
2. `main`で停止（カーネルメイン）
3. `init_device`で停止（デバイス初期化）
4. `uart_pl011_init`で停止（UART初期化）
5. `uart_pl011_putchar`で停止（文字出力）
6. QEMUのシリアル出力に文字が表示される

## トラブルシューティング

### QEMUが起動しない
- QEMUのバージョンを確認: `qemu-system-aarch64 --version`
- メモリサイズを調整: `-m 256M` → `-m 128M`

### GDBが接続できない
- QEMUが`-s -S`オプションで起動されているか確認
- ポート1234が使用可能か確認: `netstat -an | grep 1234`
- ファイアウォールの設定を確認

### ブレークポイントで停止しない
- シンボルが正しくロードされているか確認: `info symbol 0x40200000`
- アドレスが正しいか確認: `nm kernel-ram.sys | grep start`
- ブレークポイントの設定を確認: `info breakpoints`

### UARTから出力されない
- UARTレジスタの状態を確認: `x/16x 0x09000000`
- `uart_pl011_init`が呼ばれているか確認
- UARTのCRレジスタ（0x09000030）が有効になっているか確認

## デバッグコマンド

### 基本的なコマンド
- `continue` (または `c`): 実行を続ける
- `step` (または `s`): ステップイン
- `next` (または `n`): ステップオーバー
- `info registers`: レジスタの状態を表示
- `x/10i $pc`: 現在の命令を10個表示
- `print variable`: 変数の値を表示

### メモリ/レジスタの確認
- `x/16x 0x40200000`: 0x40200000から16バイトを16進数で表示
- `x/16x 0x09000000`: UARTレジスタ（PL011）の状態を確認
- `info registers x0 x1 x2`: 特定のレジスタを表示

## 次のステップ

動作確認ができたら：
1. UART出力が表示されることを確認
2. カーネルの初期化シーケンスが正常に動作することを確認
3. 必要に応じて、残りの実装項目（レジスタ取得/設定など）を実装

---

**詳細は `VERIFICATION_RESULTS.md` と `ACTION_CHECKLIST.md` を参照してください。**
