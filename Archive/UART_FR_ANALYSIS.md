# UARTFR（Flag Register）の分析

## UARTFRのビット定義

- **TXFE (bit 7)**: Transmit FIFO Empty = 1 なら、FIFOが空
- **TXFF (bit 5)**: Transmit FIFO Full = 1 なら、FIFOが満杯
- **RXFE (bit 4)**: Receive FIFO Empty = 1 なら、受信FIFOが空
- **BUSY (bit 3)**: UART Busy = 1 なら、送信中

## 現在の状態

- UARTFR = 0x90 = 0b10010000
  - TXFE (bit 7) = 1: FIFOが空
  - TXFF (bit 5) = 0: FIFOが満杯ではない
  - RXFE (bit 4) = 1: 受信FIFOが空

## 期待される動作

UARTDRにデータを書き込んだ後：
- TXFEビット（bit 7）がクリアされる（0になる）
- BUSYビット（bit 3）がセットされる（1になる）可能性がある

## 確認手順

### ステップ1: str命令実行前のUARTFRを確認
```gdb
print/x *(volatile unsigned int*)0x09000018
```

### ステップ2: str命令を実行
```gdb
stepi
```

### ステップ3: str命令実行後のUARTFRを確認
```gdb
print/x *(volatile unsigned int*)0x09000018
```

### ステップ4: メモリバリアの後も確認
```gdb
stepi
print/x *(volatile unsigned int*)0x09000018
```

## トラブルシューティング

### TXFFビットがセットされている場合
FIFOが満杯で、書き込みが受け付けられていません。

### TXFEビットがクリアされない場合
UARTDRへの書き込みが実際には行われていない可能性があります。

### BUSYビットがセットされない場合
UARTが送信状態になっていない可能性があります。
