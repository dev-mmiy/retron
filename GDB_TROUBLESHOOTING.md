# GDBトラブルシューティング

## 問題: PCレジスタが0x200を指している

### 症状
- PCレジスタが0x200を指している
- エントリーポイント（0x40200000）に到達していない
- ブレークポイントで停止しない

### 原因
カーネルが正しくロードされていない、または実行が正しく開始されていない可能性があります。

## 解決方法

### ステップ1: メモリの確認

エントリーポイントのメモリを確認：
```gdb
(gdb) x/16x 0x40200000
```

正常な場合、`start`関数のコードが表示されるはずです。

### ステップ2: エントリーポイントに移動

```gdb
(gdb) set $pc = 0x40200000
(gdb) x/10i $pc
```

これで`start`関数のコードが表示されるはずです。

### ステップ3: スタックポインタの確認

```gdb
(gdb) info registers sp
(gdb) x/16x $sp
```

スタックポインタが有効なアドレスを指しているか確認します。

### ステップ4: 実行を再開

```gdb
(gdb) continue
```

または、ステップ実行で確認：
```gdb
(gdb) stepi
(gdb) stepi
(gdb) info registers pc
```

## 確認コマンド

### メモリの確認
```gdb
# エントリーポイント
(gdb) x/16x 0x40200000
(gdb) x/10i 0x40200000

# main関数
(gdb) x/16x 0x40218000
(gdb) x/10i 0x40218000

# UARTレジスタ
(gdb) x/16x 0x09000000
```

### レジスタの確認
```gdb
(gdb) info registers
(gdb) info registers pc sp x0 x1
```

### ブレークポイントの確認
```gdb
(gdb) info breakpoints
(gdb) disable 1
(gdb) enable 1
```

## QEMUの確認

QEMUが正しくカーネルをロードしているか確認：

1. QEMUを再起動
2. GDBで再接続
3. エントリーポイントに移動して実行

## 代替方法

### 方法1: QEMUを再起動

```bash
# QEMUを停止
pkill -f qemu-system-aarch64

# QEMUを再起動
./run-tkernel-qemu.sh
```

### 方法2: 手動でロード

GDBで手動でカーネルをロード：
```gdb
(gdb) file third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
(gdb) target remote :1234
(gdb) set $pc = 0x40200000
(gdb) x/10i $pc
(gdb) continue
```
