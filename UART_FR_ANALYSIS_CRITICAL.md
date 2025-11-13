# UARTFRの矛盾した値の分析

## 重要な発見

### UARTFR = 0x90 の分析
```
UARTFR = 0x90 = 0b10010000
- bit 0 (CTS) = 0
- bit 1 (DSR) = 0
- bit 2 (DCD) = 0
- bit 3 (BUSY) = 1 ✅
- bit 4 (RXFE) = 1 ✅ (受信FIFOが空)
- bit 5 (TXFF) = 1 ❌ (送信FIFOがフル)
- bit 6 (RXFF) = 0
- bit 7 (TXFE) = 1 ❌ (送信FIFOが空)
```

### 矛盾点
- **TXFF = 1** かつ **TXFE = 1** は矛盾しています
- TXFF（送信FIFOがフル）とTXFE（送信FIFOが空）が同時に1になることはありません

### 考えられる原因

1. **UARTFRレジスタの読み取りが正しく動作していない**
   - キャッシュの問題
   - メモリマッピングの問題

2. **UARTが正しく初期化されていない**
   - UARTCR = 0x301は正常だが、UARTFRの状態が異常

3. **QEMUのUARTデバイスの問題**
   - QEMUのPL011 UARTデバイスが正しく動作していない

## 解決方法

### 方法1: UARTFRの読み取りを確認

`uart_read_reg`関数が正しく動作しているか確認：
- キャッシュの問題がないか
- メモリマッピングが正しいか

### 方法2: UART初期化を再確認

`uart_pl011_init`が正しく実行されているか確認：
- UARTCRが0x301になっているか（確認済み ✅）
- UARTFRの初期状態を確認

### 方法3: タイムアウトを追加

`uart_pl011_putchar`の無限ループを防ぐために、タイムアウトを追加：

```c
EXPORT void uart_pl011_putchar( UB c )
{
	int timeout = 1000000;  // タイムアウトカウンタ
	
	/* Wait until transmit FIFO is not full */
	while ( (uart_read_reg(0x018) & UART_FR_TXFF) && timeout > 0 ) {
		timeout--;
	}
	
	if ( timeout == 0 ) {
		/* Timeout - UART may not be ready */
		return;
	}

	/* Send character */
	uart_write_reg(0x000, (UW)c);	/* DR */
}
```

## 次のステップ

1. UARTFRの読み取りが正しく動作しているか確認
2. タイムアウトを追加して無限ループを防ぐ
3. QEMUのUARTデバイスの状態を確認

