# UART出力問題の修正提案

## 問題の整理

現在の状況：
- ✅ UARTCR = 0x301（正常）
- ✅ `str x0, [x1]`命令が実行されている
- ✅ キャッシュフラッシュ（`dc civac`）とメモリバリア（`dmb sy`）も実行されている
- ❌ QEMUのシリアル出力に何も表示されない

## 考えられる原因と解決策

### 原因1: QEMU virt machineのUARTアドレスが異なる

**確認方法**:
```bash
# QEMUのmonitorで確認
./check_uart_memory_mapping.sh
# QEMU monitorで:
(qemu) info mtree
(qemu) x/16x 0x09000000
(qemu) info qtree | grep -i uart
```

**解決策**: 実際のUARTアドレスを確認し、必要に応じて`uart_pl011.h`の`UART_PL011_BASE`を修正

### 原因2: デバイスメモリへの書き込みがキャッシュに留まっている

**確認方法**: 
- GDBで`str`命令実行後、`dc civac`実行前後でUARTDRの値を確認
- QEMUのターミナルで文字が表示されるか確認

**解決策**: キャッシュフラッシュのタイミングを調整
- 書き込みの前後でキャッシュフラッシュを実行
- または、書き込み前にキャッシュを無効化

### 原因3: QEMUのシリアル出力設定の問題

**確認方法**:
- QEMUの起動オプションを確認（`-serial stdio`が正しく設定されているか）
- 別のシリアル出力方法を試す（`-chardev stdio,id=serial0 -serial chardev:serial0`）

**解決策**: QEMUのシリアル出力設定を修正

## 推奨される修正手順

### ステップ1: QEMUのmonitorでメモリマッピングを確認（最優先）

```bash
# ターミナル1: QEMUを起動（monitor: stdio）
./check_uart_memory_mapping.sh

# QEMU monitorで以下を実行：
(qemu) info mtree
(qemu) x/16x 0x09000000
(qemu) info qtree | grep -i uart
```

### ステップ2: 実際のUARTアドレスを確認

QEMU virt machineのドキュメントまたはソースコードで、実際のUARTアドレスを確認します。

### ステップ3: キャッシュフラッシュのタイミングを調整（必要に応じて）

`uart_write_reg`関数で、書き込みの前後でキャッシュフラッシュを実行するように修正します。

### ステップ4: QEMUのシリアル出力設定を確認

`-serial stdio`が正しく動作しているか確認し、必要に応じて別の方法を試します。

## 次のアクション

1. **今すぐ実行**: QEMUのmonitorでメモリマッピングを確認
2. **次に実行**: 実際のUARTアドレスを確認
3. **必要に応じて**: キャッシュフラッシュのタイミングを調整

