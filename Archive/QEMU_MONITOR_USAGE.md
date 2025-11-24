# QEMU Monitor使用方法

## 問題

`-serial stdio`と`-monitor stdio`を同時に使用できない。

## 解決方法

### 方法1: TCPポートでmonitorを有効化（推奨）

#### ステップ1: QEMUを起動
```bash
./run-qemu-monitor-simple.sh
```

#### ステップ2: 別のターミナルでmonitorに接続
```bash
./connect_qemu_monitor_tcp.sh
```

または、直接telnetで接続：
```bash
telnet 127.0.0.1 5555
```

#### ステップ3: monitorでメモリマッピングを確認
```
(qemu) info mtree
(qemu) x/16x 0x09000000
```

### 方法2: Unixソケットでmonitorを有効化

#### ステップ1: QEMUを起動
```bash
./run-qemu-with-monitor.sh
```

#### ステップ2: 別のターミナルでmonitorに接続
```bash
./connect_qemu_monitor.sh
```

または、直接socatで接続：
```bash
socat - unix:monitor.sock
```

## monitorコマンド

### メモリマッピングを確認
```
(qemu) info mtree
```

### メモリを表示
```
(qemu) x/16x 0x09000000
```

### メモリに書き込み
```
(qemu) memwrite 0x09000000 0x48 4
```

### QEMUを終了
```
(qemu) quit
```
