# GDB次のステップ

## 現在の状態

✅ エントリーポイント（start）に到達
✅ ブレークポイント1（start）で停止
✅ 実行を続行中

## 次の確認手順

### 1. 各ブレークポイントでの確認

実行が続いているので、次のブレークポイントで停止するまで待ちます。

#### main ブレークポイントで停止したら：
```gdb
(gdb) info registers pc
(gdb) x/10i $pc
(gdb) info registers x0 x1 x2
(gdb) continue
```

#### uart_pl011_init ブレークポイントで停止したら：
```gdb
(gdb) info registers pc
(gdb) x/10i $pc
(gdb) x/16x 0x09000000
(gdb) print/x *(volatile unsigned int*)0x09000030
(gdb) continue
```

#### uart_pl011_putchar ブレークポイントで停止したら：
```gdb
(gdb) info registers pc
(gdb) print/x $x0
(gdb) x/16x 0x09000000
(gdb) print/x *(volatile unsigned int*)0x09000030
(gdb) continue
```

### 2. 実行が停止しない場合

実行が続いているが、ブレークポイントで停止しない場合：

```gdb
(gdb) interrupt
(gdb) info registers pc
(gdb) x/10i $pc
```

### 3. UART出力の確認

QEMUのターミナル（QEMUを起動したターミナル）で、シリアル出力を確認してください。

UARTレジスタを確認するには：
```gdb
(gdb) x/16x 0x09000000
```

UART Control Register (0x09000030) を確認：
```gdb
(gdb) print/x *(volatile unsigned int*)0x09000030
```

### 4. メモリの確認

カーネルのメモリ領域を確認：
```gdb
(gdb) x/16x 0x40200000
(gdb) x/16x 0x40218000
```

## 期待される動作

正常に動作している場合：
1. ✅ `start`で停止（確認済み）
2. `main`で停止（PC = 0x40218000付近）
3. `uart_pl011_init`で停止（PC = 0x402109b0付近）
4. `uart_pl011_putchar`で停止（PC = 0x40210a20付近）
5. QEMUのシリアル出力に文字が表示される

## トラブルシューティング

### ブレークポイントで停止しない
- 実行が続いている場合は、`interrupt`で停止して状態を確認
- ブレークポイントが有効か確認：`info breakpoints`

### UART出力が表示されない
- UARTレジスタの状態を確認：`x/16x 0x09000000`
- UART Control Registerを確認：`print/x *(volatile unsigned int*)0x09000030`
- `uart_pl011_init`が呼ばれているか確認

### 実行がハングしている
- `interrupt`で停止して状態を確認
- PCレジスタを確認：`info registers pc`
- スタックを確認：`x/16x $sp`
