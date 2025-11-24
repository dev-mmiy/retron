# UARTデバッグ完全ガイド（PC/SP設定含む）

## 準備

### ターミナル1: QEMUを起動
```bash
./run-tkernel-qemu.sh
```

### ターミナル2: GDBを起動
```bash
./debug-kernel.sh
```

## GDBコマンド（順番に実行）

### ステップ1: 初期設定（PC/SP設定）
```gdb
set $pc = 0x40200000
set $sp = 0x4ff00000
print/x $pc
print/x $sp
```

### ステップ2: ブレークポイント設定
```gdb
break uart_pl011_init
break uart_pl011_putchar
info breakpoints
```

### ステップ3: 実行開始
```gdb
continue
```

### ステップ4: uart_pl011_initで停止したら
```gdb
print/x *(volatile unsigned int*)0x09000030
finish
print/x *(volatile unsigned int*)0x09000030
```
**期待値**: 0x301（UARTEN=1, TXE=1, RXE=1）

### ステップ5: uart_pl011_putcharで停止したら
```gdb
print/x $x0
print/x *(volatile unsigned int*)0x09000030
print/x *(volatile unsigned int*)0x09000018
print/x *(volatile unsigned int*)0x09000000
```

### ステップ6: str命令の直前までステップ実行
```gdb
stepi
stepi
stepi
stepi
stepi
stepi
stepi
stepi
```

### ステップ7: str命令の直前でレジスタを確認
```gdb
print/x $x1
print/x $x0
print/x $pc
x/5i $pc
print/x *(volatile unsigned int*)0x09000018
```
**期待値**: 
- `$x1` = 0x09000000
- `$x0` = 送信する文字（例: 0x11）

### ステップ8: str命令を実行
```gdb
stepi
```

**注意**: もし固まったら、**Ctrl+C**で中断してから：
```gdb
print/x $pc
x/5i $pc
finish または continue
```

### ステップ9: UARTDRを確認（str命令直後）
```gdb
print/x *(volatile unsigned int*)0x09000000
print/x *(volatile unsigned int*)0x09000018
```

### ステップ10: メモリバリアの後も確認
```gdb
stepi
print/x *(volatile unsigned int*)0x09000000
print/x *(volatile unsigned int*)0x09000018
```

### ステップ11: 関数の終了まで実行
```gdb
finish
print/x *(volatile unsigned int*)0x09000000
print/x *(volatile unsigned int*)0x09000018
```

### ステップ12: QEMUのシリアル出力を確認
QEMUを起動したターミナルで文字が表示されているか確認します。

## トラブルシューティング

### stepiで固まった場合

1. **Ctrl+Cで中断**
2. **現在の状態を確認**
   ```gdb
   print/x $pc
   x/5i $pc
   print/x $x0
   print/x $x1
   ```
3. **UARTレジスタを確認**
   ```gdb
   print/x *(volatile unsigned int*)0x09000030
   print/x *(volatile unsigned int*)0x09000018
   print/x *(volatile unsigned int*)0x09000000
   ```
4. **実行を再開**
   ```gdb
   finish
   ```
   または
   ```gdb
   continue
   ```

## ファイルの使用

### 方法1: コマンドファイルを参照
`gdb_uart_complete_commands.txt`を参照して、順番にコマンドを実行します。

### 方法2: ステップバイステップ版を使用
`gdb_uart_step_by_step_complete.txt`を参照して、対話的にコマンドを実行します。
