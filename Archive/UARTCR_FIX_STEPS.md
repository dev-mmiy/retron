# UARTCR修正手順

## 問題

- UARTCR = 0x300（UARTEN=0、UARTが無効）
- UARTDRへの書き込みが反映されない（0x0のまま）

## 原因

`uart_pl011_init`が実行されていないか、または実行されたがUARTCRが正しく設定されていない可能性があります。

## 解決手順

### ステップ1: uart_pl011_initの実行を確認

現在`uart_pl011_putchar`で停止しているので、まず`uart_pl011_init`が実行されたか確認します。

```gdb
# 現在の状態を確認
(gdb) print/x *(volatile unsigned int*)0x09000030
# 0x300になっているはず

# uart_pl011_initのブレークポイントを設定（既に実行済みの可能性）
(gdb) break uart_pl011_init

# 実行を再開（uart_pl011_initが呼ばれるまで）
(gdb) continue
```

### ステップ2: uart_pl011_initをステップ実行

`uart_pl011_init`で停止したら：

```gdb
# 関数の開始時点でUARTCRを確認
(gdb) print/x *(volatile unsigned int*)0x09000030

# ステップ実行で初期化を追跡
(gdb) stepi
(gdb) stepi
(gdb) stepi
# ...（必要に応じて続ける）

# 82行目のUARTCR設定を確認
# cr = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
# uart_write_reg(0x030, cr);
(gdb) finish

# 関数終了後にUARTCRを確認（0x301になっているはず）
(gdb) print/x *(volatile unsigned int*)0x09000030
```

### ステップ3: 手動でUARTCRを有効化（必要に応じて）

もし`uart_pl011_init`が実行されていない、またはUARTCRが0x301になっていない場合：

```gdb
# 手動でUARTCRを有効化
(gdb) set *(volatile unsigned int*)0x09000030 = 0x301

# 確認
(gdb) print/x *(volatile unsigned int*)0x09000030
# 0x301になっているはず
```

### ステップ4: UARTDRにデータを書き込んでテスト

```gdb
# UARTFRを確認
(gdb) print/x *(volatile unsigned int*)0x09000018

# UARTDRに文字を書き込む
(gdb) set *(volatile unsigned int*)0x09000000 = 0x48  # 'H'

# 確認
(gdb) print/x *(volatile unsigned int*)0x09000000
# 0x48になっているはず

# QEMUのターミナルに文字が表示されるか確認
```

### ステップ5: uart_pl011_putcharを再実行

```gdb
# 実行を再開
(gdb) continue
```

## 期待される動作

正常に動作している場合：
1. `uart_pl011_init`実行後、UARTCR = 0x301
2. UARTDRにデータが書き込まれる
3. QEMUのシリアル出力に文字が表示される

## トラブルシューティング

### UARTCRが0x301にならない場合

1. **メモリバリアの問題**: `uart_write_reg`に`memory_barrier()`が追加されているか確認
2. **キャッシュの問題**: データキャッシュが有効で、書き込みがキャッシュに留まっている可能性
3. **MMUの問題**: MMUが有効で、仮想アドレスへのマッピングが必要な可能性

### UARTDRへの書き込みが反映されない場合

1. **UARTCRが有効か確認**: UARTCR = 0x301であることを確認
2. **UARTFRを確認**: TXFFビット（bit 5）がセットされていないか確認
3. **メモリマッピングを確認**: 物理アドレス0x09000000が正しくマッピングされているか確認
