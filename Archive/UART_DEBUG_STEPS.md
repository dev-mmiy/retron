# UARTデバッグ手順

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

### ステップ1: 初期設定
```gdb
set $pc = 0x40200000
set $sp = 0x4ff00000
```

### ステップ2: ブレークポイント設定
```gdb
break uart_pl011_init
break uart_pl011_putchar
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
```
**期待値**: 
- `$x1` = 0x09000000
- `$x0` = 送信する文字（例: 0x13）

### ステップ8: str命令を実行
```gdb
stepi
```

### ステップ9: UARTDRを確認（str命令直後）
```gdb
print/x *(volatile unsigned int*)0x09000000
print/x *(volatile unsigned int*)0x09000018
```
**期待値**: 
- UARTDR = 送信する文字（例: 0x13）

### ステップ10: メモリバリアの後も確認
```gdb
stepi
print/x *(volatile unsigned int*)0x09000000
```

### ステップ11: 関数の終了まで実行
```gdb
finish
print/x *(volatile unsigned int*)0x09000000
print/x *(volatile unsigned int*)0x09000018
```

## ファイルの使用

### 方法1: コマンドファイルを読み込む
```gdb
source gdb_uart_debug_commands.txt
```

### 方法2: 手動でコマンドを実行
`gdb_uart_debug_manual.txt`を参照して、対話的にコマンドを実行します。

## トラブルシューティング

### GDBが"Continuing"で止まる場合
- Ctrl+Cで中断
- ブレークポイントが設定されているか確認
- QEMUが正常に動作しているか確認

### UARTDRにデータが書き込まれない場合
- `$x1`が0x09000000であることを確認
- UARTCRが0x301であることを確認
- str命令が実際に実行されているか確認
