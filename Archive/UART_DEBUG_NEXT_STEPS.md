# UART出力問題の次のステップ

## 現在の状況

- ✅ UARTCR = 0x301（正常）
- ✅ `str x0, [x1]`命令が実行されている
- ✅ キャッシュフラッシュ（`dc civac`）とメモリバリア（`dmb sy`）も実行されている
- ❌ QEMUのシリアル出力に何も表示されない
- ❌ UARTDR = 0x0のまま（ただし、書き込み専用レジスタの可能性）

## 考えられる原因

### 1. QEMU virt machineのUARTアドレスの問題

QEMU virt machineでは、PL011 UARTのベースアドレスは通常0x09000000ですが、確認が必要です。

### 2. デバイスメモリのキャッシュ属性の問題

MMUが無効でも、データキャッシュが有効な場合、デバイスメモリへの書き込みがキャッシュに留まっている可能性があります。

### 3. QEMUのシリアル出力設定の問題

`-serial stdio`が正しく動作していない可能性があります。

## 確認手順

### ステップ1: QEMUのmonitorでメモリマッピングを確認

```bash
# ターミナル1: QEMUを起動（monitor: stdio）
./check_uart_memory_mapping.sh

# QEMU monitorで以下を実行：
(qemu) info mtree
(qemu) x/16x 0x09000000
(qemu) info qtree | grep -i uart
(qemu) info qtree | grep -i pl011
```

### ステップ2: 実際のUARTアドレスを確認

QEMU virt machineのドキュメントまたはソースコードで、実際のUARTアドレスを確認します。

一般的なアドレス：
- 0x09000000（標準）
- 0x9000000（8MB、異なる可能性）

### ステップ3: デバイスメモリへの直接書き込みテスト

GDBで直接UARTDRに書き込んで、QEMUのシリアル出力に表示されるか確認します。

```gdb
# uart_pl011_putcharで停止後
(gdb) print/x $x0  # 送信する文字
(gdb) print/x $x1  # UARTDRアドレス（0x09000000）
(gdb) stepi  # str命令を実行
(gdb) stepi  # dc civac命令を実行
(gdb) stepi  # dmb sy命令を実行
# この時点でQEMUのターミナルを確認
```

### ステップ4: キャッシュフラッシュのタイミングを調整

`uart_write_reg`関数で、書き込みの前後でキャッシュフラッシュを実行するように修正します。

## 解決方法

### 方法1: UARTアドレスの確認と修正

QEMUのmonitorで実際のUARTアドレスを確認し、必要に応じて修正します。

### 方法2: キャッシュフラッシュのタイミングを調整

書き込みの前後でキャッシュフラッシュを実行するように修正します。

### 方法3: QEMUのシリアル出力設定を確認

`-serial stdio`が正しく動作しているか確認し、必要に応じて別の方法を試します。

## 次のアクション

1. **今すぐ実行**: QEMUのmonitorでメモリマッピングを確認
2. **次に実行**: 実際のUARTアドレスを確認
3. **必要に応じて**: キャッシュフラッシュのタイミングを調整

