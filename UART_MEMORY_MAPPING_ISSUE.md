# UARTメモリマッピング問題

## 問題

UARTCRとUARTDRへの書き込みが反映されない：
- `set *(volatile unsigned int*)0x09000030 = 0x301` を実行しても、0x300のまま
- `set *(volatile unsigned int*)0x09000000 = 0x48` を実行しても、0x0のまま

## 考えられる原因

1. **メモリマッピングの問題**
   - QEMUのメモリマッピングが正しく設定されていない
   - 物理アドレスと仮想アドレスのマッピングが不一致

2. **キャッシュの問題**
   - データキャッシュが有効で、書き込みがキャッシュに留まっている
   - メモリバリアが必要

3. **MMUの問題**
   - MMUが有効で、仮想アドレスへのマッピングが必要
   - ただし、USE_MMU=0のはずなので、MMUは無効

4. **QEMUの設定問題**
   - QEMUのメモリマッピングが正しく設定されていない

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

### ステップ2: メモリバリアを追加

uart_pl011.cの`uart_write_reg`関数にメモリバリアを追加する必要があるかもしれません。

### ステップ3: 物理アドレスの確認

QEMU virt machineでは、UARTの物理アドレスは0x09000000です。
MMUが無効な場合、このアドレスに直接アクセスできるはずです。

## 解決方法

### 方法1: メモリバリアを追加

`uart_pl011.c`の`uart_write_reg`関数を修正：
```c
Inline void uart_write_reg( UW offset, UW value )
{
	volatile UW *reg = (volatile UW *)(UART_PL011_BASE + offset);
	*reg = value;
	memory_barrier();  // メモリバリアを追加
}
```

### 方法2: キャッシュを無効化

データキャッシュが有効な場合、キャッシュを無効化する必要があるかもしれません。

### 方法3: QEMUの設定を確認

QEMUのメモリマッピングが正しく設定されているか確認。
