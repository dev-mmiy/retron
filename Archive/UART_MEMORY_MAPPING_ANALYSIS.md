# UARTメモリマッピング分析結果

## 確認結果（2024-11-09）

### ✅ UARTアドレスの確認
- **UARTアドレスは正しい**: `0000000009000000-0000000009000fff (prio 0, i/o): pl011`
- 0x09000000が正しいベースアドレスです

### ❌ 問題の発見

#### UARTレジスタの状態（QEMU monitorで確認）
```
0x09000000 (UARTDR): 0x00000000
0x09000018 (UARTFR): 0x00000090
0x09000030 (UARTCR): 0x00000300  ← 問題！
```

#### 問題の詳細
- **UARTCR = 0x300**: UARTENビット（bit 0）が0になっている
- **UARTが無効になっている**: 正常な値は0x301（UARTEN=1, TXE=1, RXE=1）であるべき
- **UARTFR = 0x90**: TXFF=1（送信FIFOがフル）、TXFE=0、RXFE=1（受信FIFOが空）

### 考えられる原因

1. **uart_pl011_initが実行されていない**
   - 初期化関数が呼ばれていない可能性

2. **uart_pl011_initの書き込みが反映されていない**
   - キャッシュの問題
   - メモリバリアの問題

3. **初期化後に何かがUARTCRを無効にしている**
   - 他のコードがUARTCRを書き換えている可能性

## 解決方法

### ステップ1: uart_pl011_initの実行を確認

GDBで以下を実行：
```gdb
(gdb) break uart_pl011_init
(gdb) continue
(gdb) stepi  # ステップ実行
(gdb) finish  # 関数終了まで
(gdb) print/x *(volatile unsigned int*)0x09000030  # UARTCRを確認
```

### ステップ2: UARTCRへの書き込みを確認

uart_pl011_init内でUARTCRへの書き込みをステップ実行：
```gdb
(gdb) break uart_write_reg
(gdb) continue
(gdb) print/x $x0  # offset (0x030)
(gdb) print/x $x1  # value (0x301)
(gdb) stepi  # str命令を実行
(gdb) stepi  # dc civac命令を実行
(gdb) stepi  # dmb sy命令を実行
(gdb) print/x *(volatile unsigned int*)0x09000030  # UARTCRを確認
```

### ステップ3: 初期化後のUARTCRを確認

uart_pl011_init実行後、UARTCRが0x301になっているか確認：
```gdb
(gdb) break uart_pl011_putchar
(gdb) continue
(gdb) print/x *(volatile unsigned int*)0x09000030  # UARTCRを確認
```

## 次のアクション

1. **今すぐ実行**: GDBでuart_pl011_initの実行を確認
2. **次に実行**: UARTCRへの書き込みをステップ実行して確認
3. **必要に応じて**: 初期化後にUARTCRが無効になっていないか確認

