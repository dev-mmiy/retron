# QEMUシリアル出力確認ガイド

## QEMUのシリアル出力について

QEMUの`-serial stdio`オプションは、シリアル出力を標準出力（stdout）に接続します。
つまり、**QEMUを起動したターミナルに直接出力が表示されます**。

## シリアル出力の確認方法

### 方法1: QEMUを起動したターミナルで確認

QEMUを起動したターミナル（`./run-tkernel-qemu.sh`を実行したターミナル）で、文字が表示されるか確認します。

**注意**: QEMUは`-S`オプションで停止状態で起動するため、GDBで接続して実行を開始するまで、何も表示されません。

### 方法2: GDBで手動テスト

GDBで接続して、手動でUARTDRにデータを書き込んでテスト：

```gdb
# 1. UARTCRを確認（0x301であることを確認）
(gdb) print/x *(volatile unsigned int*)0x09000030

# 2. 手動でUARTDRに文字を書き込む
(gdb) set *(volatile unsigned int*)0x09000000 = 0x48  # 'H'
(gdb) print/x *(volatile unsigned int*)0x09000000

# 3. QEMUのターミナルに文字が表示されるか確認
```

### 方法3: uart_pl011_putcharの実行を確認

```gdb
# 1. uart_pl011_putcharで停止
(gdb) break uart_pl011_putchar
(gdb) continue

# 2. 引数を確認
(gdb) print/x $x0
(gdb) print/c $x0

# 3. 関数を実行
(gdb) finish

# 4. UARTDRにデータが書き込まれたか確認
(gdb) print/x *(volatile unsigned int*)0x09000000

# 5. QEMUのターミナルに文字が表示されるか確認
```

## 期待される動作

正常に動作している場合：
1. GDBで`set *(volatile unsigned int*)0x09000000 = 0x48`を実行
2. QEMUを起動したターミナルに`H`が表示される
3. または、`uart_pl011_putchar`が実行されると、QEMUのターミナルに文字が表示される

## トラブルシューティング

### 何も表示されない場合

1. **UARTCRを確認**
   ```gdb
   (gdb) print/x *(volatile unsigned int*)0x09000030
   ```
   0x301になっているか確認

2. **UARTDRにデータが書き込まれているか確認**
   ```gdb
   (gdb) print/x *(volatile unsigned int*)0x09000000
   ```
   0以外の値になっているか確認

3. **UARTFRを確認**
   ```gdb
   (gdb) print/x *(volatile unsigned int*)0x09000018
   ```
   TXFEビット（bit 7）がクリアされているか確認

4. **QEMUの設定を確認**
   - `-serial stdio`が正しく設定されているか
   - ターミナルのバッファリングを確認

5. **QEMUを再起動**
   - QEMUを停止して再起動
   - シリアル出力が正しく接続されているか確認

### ターミナルのバッファリング

ターミナルがバッファリングしている場合、`stdbuf`を使用：
```bash
stdbuf -o0 ./run-tkernel-qemu.sh
```

または、QEMU起動スクリプトを修正：
```bash
stdbuf -o0 ${QEMU} ...
```

## テスト手順

1. **ターミナル1**: QEMUを起動
   ```bash
   ./run-tkernel-qemu.sh
   ```

2. **ターミナル2**: GDBで接続
   ```bash
   ./debug-kernel.sh
   ```

3. **GDBで初期設定**
   ```gdb
   (gdb) set $pc = 0x40200000
   (gdb) set $sp = 0x4ff00000
   (gdb) continue
   ```

4. **手動テスト**
   ```gdb
   (gdb) set *(volatile unsigned int*)0x09000000 = 0x48
   ```

5. **ターミナル1を確認**: `H`が表示されるか確認
