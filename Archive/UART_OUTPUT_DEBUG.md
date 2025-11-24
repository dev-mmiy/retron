# UART出力デバッグガイド

## 問題

QEMUを起動したターミナルに何も表示されない。

## 確認手順

### ステップ1: uart_pl011_putcharブレークポイントで確認

`uart_pl011_putchar`ブレークポイントで停止したら：

```gdb
(gdb) print/x $x0
(gdb) print/c $x0
```

`x0`レジスタに送信する文字が入っているか確認します。

### ステップ2: UARTレジスタの状態を確認

```gdb
(gdb) x/16x 0x09000000
(gdb) print/x *(volatile unsigned int*)0x09000000
(gdb) print/x *(volatile unsigned int*)0x09000018
(gdb) print/x *(volatile unsigned int*)0x09000030
```

- UARTDR (0x09000000): データレジスタ
- UARTFR (0x09000018): フラグレジスタ
- UARTCR (0x09000030): 制御レジスタ

### ステップ3: ステップ実行でUARTレジスタへの書き込みを確認

```gdb
(gdb) stepi
(gdb) stepi
(gdb) x/16x 0x09000000
(gdb) print/x *(volatile unsigned int*)0x09000000
```

UART Data Registerにデータが書き込まれているか確認します。

### ステップ4: UART Flag Registerを確認

```gdb
(gdb) print/x *(volatile unsigned int*)0x09000018
```

- TXFE (bit 7): TX FIFO Empty
- TXFF (bit 5): TX FIFO Full
- BUSY (bit 3): UART Busy

## 期待される動作

正常に動作している場合：
1. `x0`レジスタに送信する文字（ASCIIコード）が入っている
2. `uart_pl011_putchar`実行後、UARTDRにデータが書き込まれる
3. UARTFRのTXFEビットがクリアされる
4. QEMUのシリアル出力に文字が表示される

## トラブルシューティング

### UARTDRにデータが書き込まれていない場合

1. `uart_pl011_putchar`の実装を確認
2. UART初期化が正しく行われているか確認
3. UART Control Registerが有効か確認

### UARTDRにデータが書き込まれているが表示されない場合

1. QEMUのシリアル出力設定を確認
2. `-serial stdio`オプションが正しく設定されているか確認
3. ターミナルの設定を確認
