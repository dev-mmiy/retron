# uart_pl011_putcharデバッグ

## 問題

- `uart_pl011_putchar`が呼ばれている（`$x0 = 0x11`）
- `finish`で関数を実行した後も、UARTDR = 0x0 のまま
- UARTFR = 0x90（TXFE=1, RXFE=1）

## 考えられる原因

### 1. UARTCRが0x301になっていない可能性
UARTCRが0x301になっていない場合、UARTDRへの書き込みが無効になります。

### 2. uart_write_reg関数が正しく実行されていない可能性
`uart_write_reg`関数が正しく実行されていない可能性があります。

### 3. メモリバリアの問題
メモリバリアが正しく動作していない可能性があります。

## 確認手順

### ステップ1: UARTCRを確認

```gdb
(gdb) break uart_pl011_putchar
(gdb) continue
(gdb) print/x *(volatile unsigned int*)0x09000030  # UARTCR（0x301であることを確認）
```

### ステップ2: uart_pl011_putcharをステップ実行

```gdb
(gdb) print/x $x0  # 送信する文字
(gdb) print/x *(volatile unsigned int*)0x09000018  # UARTFR（書き込み前）
(gdb) print/x *(volatile unsigned int*)0x09000000  # UARTDR（書き込み前）
(gdb) stepi
(gdb) stepi
(gdb) stepi
(gdb) stepi
(gdb) stepi
(gdb) stepi
(gdb) stepi
(gdb) stepi
(gdb) print/x *(volatile unsigned int*)0x09000000  # UARTDR（書き込み後）
(gdb) print/x *(volatile unsigned int*)0x09000018  # UARTFR（書き込み後）
```

### ステップ3: uart_write_reg関数の実行を確認

`uart_write_reg`はインライン関数なので、アセンブリコードを確認します：

```gdb
(gdb) disassemble uart_pl011_putchar
```

メモリへの書き込み命令（`str`）が生成されているか確認します。
