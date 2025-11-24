# GDB動作確認ガイド

## 現在の状態

✅ GDBがQEMUに接続されました
✅ ブレークポイントが設定されました：
- `start` (0x40200004)
- `main` (0x40218008)
- `uart_pl011_init` (0x402109b4)
- `uart_pl011_putchar` (0x40210a20)

## テスト手順

### ステップ1: エントリーポイントに移動

現在のPCレジスタを確認し、エントリーポイントに移動します：

```gdb
(gdb) info registers pc
(gdb) set $pc = 0x40200000
(gdb) x/10i $pc
```

### ステップ2: 実行を開始

```gdb
(gdb) continue
```

または短縮形：
```gdb
(gdb) c
```

### ステップ3: ブレークポイントでの停止確認

`start`ブレークポイントで停止したら：

```gdb
(gdb) info registers
(gdb) x/10i $pc
(gdb) continue
```

### ステップ4: 各ブレークポイントでの確認

#### start ブレークポイント
```gdb
(gdb) info registers pc
(gdb) x/10i $pc
(gdb) print/x $x0
(gdb) continue
```

#### main ブレークポイント
```gdb
(gdb) info registers pc
(gdb) x/10i $pc
(gdb) continue
```

#### uart_pl011_init ブレークポイント
```gdb
(gdb) info registers pc
(gdb) x/10i $pc
(gdb) x/16x 0x09000000
(gdb) continue
```

#### uart_pl011_putchar ブレークポイント
```gdb
(gdb) info registers pc
(gdb) x/10i $pc
(gdb) print/x $x0
(gdb) x/16x 0x09000000
(gdb) continue
```

## 確認ポイント

### 1. エントリーポイントの確認
- PCレジスタが0x40200000を指しているか
- `start`関数のコードが表示されるか

### 2. 初期化シーケンスの確認
- `main`が呼ばれるか
- `init_device`が呼ばれるか
- `uart_pl011_init`が呼ばれるか

### 3. UARTレジスタの確認
```gdb
(gdb) x/16x 0x09000000
```

UARTレジスタの状態を確認：
- 0x09000000: UARTDR (Data Register)
- 0x09000004: UARTRSR/UARTECR (Receive Status/Error Clear)
- 0x09000018: UARTFR (Flag Register)
- 0x09000030: UARTCR (Control Register)

### 4. メモリの確認
```gdb
(gdb) x/16x 0x40200000
(gdb) x/16x 0x40218000
```

## 期待される動作

正常に動作している場合：
1. `start`で停止（PC = 0x40200000付近）
2. `main`で停止（PC = 0x40218000付近）
3. `uart_pl011_init`で停止（PC = 0x402109b0付近）
4. `uart_pl011_putchar`で停止（PC = 0x40210a20付近）

## トラブルシューティング

### PCレジスタが0x200を指している
- QEMUがまだエントリーポイントに到達していない可能性があります
- `set $pc = 0x40200000`でエントリーポイントに移動してから`continue`

### ブレークポイントで停止しない
- シンボルが正しくロードされているか確認：`info symbol 0x40200000`
- ブレークポイントの状態を確認：`info breakpoints`

### UARTレジスタが0になっている
- UART初期化がまだ実行されていない可能性があります
- `uart_pl011_init`ブレークポイントで停止するまで`continue`

## 便利なコマンド

```gdb
# レジスタの表示
(gdb) info registers
(gdb) info registers x0 x1 x2

# メモリの表示
(gdb) x/16x 0x40200000
(gdb) x/10i $pc

# ブレークポイントの管理
(gdb) info breakpoints
(gdb) delete 1
(gdb) disable 2
(gdb) enable 2

# 実行制御
(gdb) continue
(gdb) step
(gdb) next
(gdb) finish
```

## 次のステップ

動作確認ができたら：
1. UART出力が表示されることを確認
2. カーネルの初期化シーケンスが正常に動作することを確認
3. 必要に応じて、残りの実装項目を実装
