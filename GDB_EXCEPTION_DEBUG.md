# GDB例外デバッグガイド

## 現在の状態

⚠️ PCが`exception_vector_table`を指している
- 例外が発生した可能性があります
- UARTレジスタは正常に初期化されている（UARTCR = 0x301）

## 確認手順

### ステップ1: 例外レジスタを確認

```gdb
(gdb) print/x $esr_el1
(gdb) print/x $far_el1
(gdb) print/x $elr_el1
```

### ステップ2: 例外の種類を特定

ESR_EL1のECフィールド（bits 31:26）で例外クラスを確認：
- 0x00: Unknown reason
- 0x15: SVC (Supervisor Call)
- 0x20: IRQ (Interrupt Request)
- 0x24: FIQ (Fast Interrupt Request)
- 0x25: System Error
- 0x2C: Data Abort (lower EL)
- 0x2D: Data Abort (same EL)
- 0x2E: Instruction Abort (lower EL)
- 0x2F: Instruction Abort (same EL)

### ステップ3: スタックとレジスタを確認

```gdb
(gdb) info registers
(gdb) info registers sp
(gdb) x/16x $sp
```

### ステップ4: バックトレースを確認

```gdb
(gdb) backtrace
(gdb) info frame
```

## UARTレジスタの状態

確認されたUARTレジスタ：
- UARTCR (0x09000030) = 0x301 ✅
  - UARTEN = 1 (UART有効)
  - TXE = 1 (TX有効)
  - RXE = 1 (RX有効)
- UARTFR (0x09000018) = 0x90
  - BUSY = 1 (送信中)
  - TXFE = 1 (TX FIFO空)

## トラブルシューティング

### 例外が発生している場合

1. 例外の種類を確認
2. 例外ハンドラが正しく実装されているか確認
3. 例外が発生した原因を調査

### UART出力が表示されない場合

1. UARTレジスタの状態を確認
2. QEMUのシリアル出力設定を確認
3. UART初期化が正しく行われているか確認
