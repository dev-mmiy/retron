# UART書き込み成功の分析

## 重要な発見

### ✅ 成功したこと
- `uart_pl011_init`実行後、UARTCR = 0x301 になっている
- カーネルコード（`uart_write_reg`関数）からの書き込みは正常に動作している

### ❌ GDBからの直接書き込みの問題
- GDBからの直接書き込み（`set *(volatile unsigned int*)0x09000030 = 0x301`）は反映されない
- これは、QEMUのUARTデバイスがGDBからの書き込みを無視しているか、または何らかの保護機構があることを示している

## 結論

**カーネルコードからの書き込みは正常に動作している**ので、UARTCRは0x301に設定されています。
次のステップは、`uart_pl011_putchar`でUARTDRにデータが書き込まれるか確認することです。

## 次の確認手順

### ステップ1: uart_pl011_putcharの実行を確認

```gdb
(gdb) break uart_pl011_putchar
(gdb) continue
(gdb) print/x $x0  # 送信する文字
(gdb) print/x *(volatile unsigned int*)0x09000000  # UARTDR（書き込み前）
(gdb) finish
(gdb) print/x *(volatile unsigned int*)0x09000000  # UARTDR（書き込み後）
(gdb) print/x *(volatile unsigned int*)0x09000018  # UARTFR
```

### ステップ2: QEMUのシリアル出力を確認

QEMUを起動したターミナルに文字が表示されるか確認します。

## 注意事項

`gic_init`で無限ループしているように見えますが、これは別の問題です。
まずはUARTの動作を確認しましょう。
