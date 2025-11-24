# QEMUシリアル出力確認ガイド

## 問題

UARTCR = 0x301（正常）だが、QEMUのシリアル出力に何も表示されない。

## 確認手順

### ステップ1: UARTDRへの書き込みを確認

`uart_pl011_putchar`ブレークポイントで停止したら：

```gdb
(gdb) print/x $x0
(gdb) print/c $x0
(gdb) print/x *(volatile unsigned int*)0x09000000
(gdb) finish
(gdb) print/x *(volatile unsigned int*)0x09000000
(gdb) print/x *(volatile unsigned int*)0x09000018
```

### ステップ2: UARTFRの状態を確認

```gdb
(gdb) print/x *(volatile unsigned int*)0x09000018
```

- TXFE (bit 7): TX FIFO Empty = 1 なら、FIFOが空
- TXFF (bit 5): TX FIFO Full = 1 なら、FIFOが満杯
- BUSY (bit 3): UART Busy = 1 なら、送信中

### ステップ3: QEMUのシリアル出力設定を確認

QEMU起動スクリプトを確認：
- `-serial stdio`: シリアル出力を標準出力に接続
- `-nographic`: グラフィック出力を無効化

### ステップ4: 手動でUARTDRにデータを書き込んでテスト

```gdb
(gdb) set *(volatile unsigned int*)0x09000000 = 0x48  # 'H'
(gdb) print/x *(volatile unsigned int*)0x09000000
```

QEMUのターミナルに文字が表示されるか確認します。

## トラブルシューティング

### UARTDRにデータが書き込まれているが表示されない場合

1. **QEMUのシリアル出力設定を確認**
   - `-serial stdio`が正しく設定されているか
   - ターミナルのバッファリングを確認

2. **UARTFRの状態を確認**
   - TXFEビットがクリアされているか
   - BUSYビットがセットされているか

3. **QEMUを再起動**
   - QEMUを停止して再起動
   - シリアル出力が正しく接続されているか確認

### UARTDRにデータが書き込まれていない場合

1. `uart_pl011_putchar`の実装を確認
2. ステップ実行でUARTDRへの書き込みを確認
3. メモリバリアが正しく動作しているか確認
