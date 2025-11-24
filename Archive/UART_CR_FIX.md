# UART Control Register修正

## 問題

UARTCR = 0x300 (UARTEN=0, 無効)
- UARTが無効になっているため、UARTDRにデータが書き込まれない
- 正常値: UARTCR = 0x301 (UARTEN=1, TXE=1, RXE=1)

## 原因

`uart_pl011_init`が呼ばれた後、UARTCRが0x300になっている可能性があります。
- 初期化が正しく完了していない
- または、何かがUARTCRを無効にしている

## 解決方法

### 方法1: GDBでUARTCRを有効化

```gdb
(gdb) set *(volatile unsigned int*)0x09000030 = 0x301
(gdb) print/x *(volatile unsigned int*)0x09000030
```

### 方法2: uart_pl011_initを再実行

```gdb
(gdb) break uart_pl011_init
(gdb) continue
(gdb) finish
(gdb) print/x *(volatile unsigned int*)0x09000030
```

## 確認手順

### ステップ1: UARTCRを有効化

```gdb
(gdb) set *(volatile unsigned int*)0x09000030 = 0x301
(gdb) print/x *(volatile unsigned int*)0x09000030
```

### ステップ2: UARTFRを確認

```gdb
(gdb) print/x *(volatile unsigned int*)0x09000018
```

### ステップ3: uart_pl011_putcharを再度実行

```gdb
(gdb) continue
```

または、手動でUARTDRにデータを書き込んでテスト：
```gdb
(gdb) set *(volatile unsigned int*)0x09000000 = 0x48  # 'H'
(gdb) print/x *(volatile unsigned int*)0x09000000
```

## 期待される動作

正常に動作している場合：
1. UARTCR = 0x301 (UARTEN=1, TXE=1, RXE=1)
2. UARTDRにデータが書き込まれる
3. QEMUのシリアル出力に文字が表示される
