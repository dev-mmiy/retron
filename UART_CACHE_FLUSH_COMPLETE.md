# UARTキャッシュフラッシュ修正完了

## 修正内容

`uart_pl011.c`の`uart_write_reg`関数にキャッシュフラッシュ（`dc civac`）を追加しました。

### 修正後のコード
```c
Inline void uart_write_reg( UW offset, UW value )
{
	volatile UW *reg = (volatile UW *)(UART_PL011_BASE + offset);
	UW addr = (UW)reg;
	*reg = value;
	/* Flush cache for device memory to ensure write reaches hardware */
	Asm("dc civac, %0":: "r"(addr): "memory");
	memory_barrier();	/* Ensure write is completed */
}
```

## 理由

ARM AArch64では、データキャッシュが有効な場合、デバイスメモリ（UARTレジスタ）への書き込みがキャッシュに留まることがあります。
`dc civac`（Clean and Invalidate by Virtual Address to Point of Coherency）命令を使用して、キャッシュをフラッシュすることで、書き込みが実際のハードウェアに到達することを保証します。

## 次のステップ

1. ✅ カーネルを再ビルド（完了）
2. QEMUとGDBで再テスト
3. UARTDRへの書き込みがQEMUのシリアル出力に反映されるか確認

## テスト手順

1. QEMUを起動: `./run-tkernel-qemu.sh`
2. GDBで接続: `./debug-kernel.sh`
3. `gdb_uart_step_by_step_complete.txt`を参照して、順番にコマンドを実行
4. QEMUのシリアル出力に文字が表示されるか確認
