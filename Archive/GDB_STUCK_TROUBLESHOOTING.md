# GDBが固まった場合の対処法

## 問題

`stepi`コマンドでGDBが固まってしまった。

## 対処法

### ステップ1: 中断する
**Ctrl+C**を押してGDBを中断します。

### ステップ2: 現在の状態を確認
```gdb
# 現在のPCを確認
print/x $pc

# 現在の命令を確認
x/5i $pc

# レジスタの状態を確認
print/x $x0
print/x $x1
```

### ステップ3: UARTレジスタの状態を確認
```gdb
# UARTCR（制御レジスタ）
print/x *(volatile unsigned int*)0x09000030

# UARTFR（フラグレジスタ）
print/x *(volatile unsigned int*)0x09000018

# UARTDR（データレジスタ）
print/x *(volatile unsigned int*)0x09000000
```

### ステップ4: 実行を再開
```gdb
# finishで関数の終了まで実行
finish

# または、continueで実行を再開
continue
```

## 考えられる原因

### 1. 無限ループ
`uart_pl011_putchar`関数内の`while`ループで無限ループしている可能性があります。

### 2. ハードウェア待機
UARTデバイスが応答を待っている可能性があります。

### 3. QEMUの問題
QEMUのUARTデバイスが応答していない可能性があります。

## 確認手順

### ステップ1: アセンブリコードを確認
```gdb
disassemble uart_pl011_putchar
```

### ステップ2: 現在の命令を確認
```gdb
x/5i $pc
```

### ステップ3: UARTFRの状態を確認
```gdb
print/x *(volatile unsigned int*)0x09000018
```

TXFFビット（bit 5）がセットされている場合、FIFOが満杯で待機している可能性があります。

## 解決方法

### 方法1: finishで関数の終了まで実行
```gdb
finish
```

### 方法2: continueで実行を再開
```gdb
continue
```

### 方法3: ブレークポイントを設定してcontinue
```gdb
break uart_pl011_putchar
continue
```
