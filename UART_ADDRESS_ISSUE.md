# UARTアドレス問題

## 問題

アセンブリコードを見ると：
- `0x40210a48`: `mov x1, #0x9000000` - これは0x9000000（9MB）を設定している
- `0x40210a50`: `str x0, [x1]` - これがUARTDRへの書き込み命令

しかし、UARTDRが0x0のままです。

## 考えられる原因

### 1. アドレスの問題
`x1`の値が正しくない可能性があります。
- 期待値: 0x09000000
- 実際の値: 0x9000000（アセンブリコードから）

### 2. メモリマッピングの問題
0x9000000への書き込みが正しくマッピングされていない可能性があります。

## 確認手順

### ステップ1: x1レジスタの値を確認

```gdb
(gdb) break uart_pl011_putchar
(gdb) continue
(gdb) stepi  # str命令の直前まで実行
(gdb) print/x $x1  # x1の値を確認（0x09000000であるべき）
(gdb) stepi  # str命令を実行
(gdb) print/x *(volatile unsigned int*)0x09000000  # UARTDRを確認
```

### ステップ2: 0x9000000への書き込みを確認

```gdb
(gdb) print/x *(volatile unsigned int*)0x9000000  # 0x9000000の値を確認
(gdb) set *(volatile unsigned int*)0x9000000 = 0x13
(gdb) print/x *(volatile unsigned int*)0x9000000  # 書き込みが反映されるか確認
```

### ステップ3: アセンブリコードの確認

`uart_write_reg`関数が正しくインライン展開されているか確認します。
