# UART書き込み問題の分析

## 発見事項

### ✅ 正常に動作していること
- `0x40200000`への書き込みは正常に反映される
- UARTレジスタの読み取りは正常（`0x09000018` = `0x90`, `0x09000030` = `0x300`）

### ❌ 問題
- `0x09000030`（UARTCR）への書き込みが反映されない（`0x301`を書き込んでも`0x300`のまま）

## 考えられる原因

### 1. QEMUのUARTデバイスの問題
QEMUのUARTデバイスが書き込みを受け付けていない可能性があります。
- QEMUのUARTデバイスが初期化されていない
- QEMUのUARTデバイスが読み取り専用になっている

### 2. メモリマッピングの問題
UARTレジスタのメモリマッピングが正しく設定されていない可能性があります。
- ただし、読み取りは正常なので、マッピング自体は存在している

### 3. カーネルコードからの書き込みを確認
GDBからの直接書き込みではなく、カーネルコード（`uart_write_reg`関数）からの書き込みを確認する必要があります。

## 確認手順

### ステップ1: uart_pl011_initの実行を確認

```gdb
(gdb) break uart_pl011_init
(gdb) continue
(gdb) stepi
(gdb) stepi
...
(gdb) finish
(gdb) print/x *(volatile unsigned int*)0x09000030
```

### ステップ2: uart_write_reg関数の実行を確認

```gdb
(gdb) break uart_write_reg
(gdb) continue
(gdb) print/x $x0  # offset
(gdb) print/x $x1  # value
(gdb) stepi
(gdb) stepi
(gdb) print/x *(volatile unsigned int*)0x09000030
```

### ステップ3: アセンブリレベルで確認

`uart_write_reg`関数のアセンブリコードを確認し、実際にメモリへの書き込みが行われているか確認します。

## 解決方法

### 方法1: カーネルコードからの書き込みを確認
GDBからの直接書き込みではなく、カーネルコード（`uart_write_reg`関数）からの書き込みを確認します。

### 方法2: QEMUの設定を確認
QEMUのUARTデバイスの設定を確認し、必要に応じて修正します。

### 方法3: メモリバリアの追加
`uart_write_reg`関数にメモリバリアを追加します（既に追加済み）。

## 次のステップ

1. `uart_pl011_init`の実行を確認
2. `uart_write_reg`関数の実行を確認
3. アセンブリレベルでメモリへの書き込みを確認
