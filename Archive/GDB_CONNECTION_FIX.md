# GDB接続問題の修正

## 問題

GDBがQEMUに接続できない、またはタイムアウトが発生している：
- "Remote replied unexpectedly to 'vMustReplyEmpty': timeout"
- "The program has no registers now."

## 原因

1. QEMUが応答していない
2. QEMUとの接続が切断されている
3. QEMUがクラッシュしている

## 解決方法

### ステップ1: QEMUの状態を確認

```bash
ps aux | grep qemu-system-aarch64
netstat -tlnp | grep :1234
```

### ステップ2: QEMUを再起動

```bash
# QEMUを停止
pkill -9 -f qemu-system-aarch64

# QEMUを再起動（別のターミナルで）
./run-tkernel-qemu.sh
```

### ステップ3: GDBで再接続

```bash
# GDBを起動
./debug-kernel.sh
```

または、手動で：
```gdb
(gdb) target remote :1234
(gdb) set $pc = 0x40200000
(gdb) set $sp = 0x4ff00000
(gdb) continue
```

## 注意事項

- QEMUとGDBは別々のターミナルで実行する必要があります
- QEMUが起動していることを確認してからGDBで接続してください
- 接続に失敗する場合は、QEMUを再起動してください
