# GDBスタック修正手順

## 問題

スタックポインタ（sp）が0x0になっているため、カーネルが正常に実行できません。

## 解決方法

### ステップ1: スタックポインタを設定

QEMU virt machineのメモリマップ：
- RAM: 0x40000000 - 0x50000000 (256MB)
- スタックは通常、RAMの上位アドレスに設定

```gdb
(gdb) set $sp = 0x4ff00000
(gdb) info registers sp
```

### ステップ2: スタックメモリを確認

```gdb
(gdb) x/16x $sp
```

スタックメモリが有効か確認します。

### ステップ3: 実行を開始

```gdb
(gdb) continue
```

または、ステップ実行で確認：
```gdb
(gdb) stepi
(gdb) stepi
(gdb) info registers pc sp
```

## 期待される動作

正常に動作している場合：
1. スタックポインタが有効なアドレス（0x4ff00000付近）を指す
2. `start`関数が正常に実行される
3. `main`ブレークポイントで停止する
4. その後、`uart_pl011_init`、`uart_pl011_putchar`の順にブレークポイントで停止する

## トラブルシューティング

### スタックポインタを設定しても実行が進まない場合

1. 他のレジスタも確認：
```gdb
(gdb) info registers
```

2. ステップ実行で確認：
```gdb
(gdb) stepi
(gdb) info registers pc sp
(gdb) x/10i $pc
```

3. メモリの状態を確認：
```gdb
(gdb) x/16x 0x40200000
(gdb) x/16x $sp
```

### 実行がハングしている場合

`interrupt`で停止して状態を確認：
```gdb
(gdb) interrupt
(gdb) info registers
(gdb) x/10i $pc
```
