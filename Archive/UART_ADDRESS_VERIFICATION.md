# UARTアドレス検証

## 問題

キャッシュフラッシュ（`dc civac`）を追加しても、UARTDRへの書き込みが反映されません。

## 確認事項

### 1. QEMU virt machineのPL011 UARTベースアドレス

QEMU virt machineでは、PL011 UARTのベースアドレスは通常0x09000000です。
しかし、実際のマッピングを確認する必要があります。

### 2. メモリマッピングの確認

QEMUのmonitorを使用して、メモリマッピングを確認します：

```bash
qemu-system-aarch64 -machine virt -cpu cortex-a53 -m 256M -nographic -serial stdio -monitor stdio
```

QEMUのmonitorで：
```
(qemu) info mtree
(qemu) x/16x 0x09000000
```

### 3. 代替アドレスの確認

QEMU virt machineでは、PL011 UARTが別のアドレスにマッピングされている可能性があります。
一般的なアドレス：
- 0x09000000（標準）
- 0x9000000（8MB、異なる可能性）

## 解決方法

### 方法1: QEMUのmonitorで確認

QEMUのmonitorを使用して、実際のUARTアドレスを確認します。

### 方法2: デバイスツリーを確認

QEMU virt machineのデバイスツリーを確認して、UARTのベースアドレスを確認します。

### 方法3: QEMUのソースコードを確認

QEMUのソースコードで、PL011 UARTのベースアドレスを確認します。
