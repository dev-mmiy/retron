#!/bin/bash
# GDBデバッグスクリプト for T-Kernel on QEMU

KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

# GDBコマンドを検出（aarch64-linux-gnu-gdb または gdb-multiarch）
if command -v aarch64-linux-gnu-gdb >/dev/null 2>&1; then
    GDB=aarch64-linux-gnu-gdb
elif command -v gdb-multiarch >/dev/null 2>&1; then
    GDB=gdb-multiarch
else
    echo "❌ GDBがインストールされていません"
    echo "   インストール方法: sudo apt-get install gdb-multiarch"
    exit 1
fi

echo "=========================================="
echo "T-Kernel GDB Debug Script"
echo "=========================================="
echo ""
echo "This script will start GDB and connect to QEMU."
echo "Make sure QEMU is running with -s -S options."
echo ""
echo "Kernel: $KERNEL"
echo ""

# GDBコマンドファイルを作成
cat > /tmp/gdb_commands.txt << 'EOF'
# GDB initialization commands for T-Kernel debugging

# ターゲットに接続
target remote :1234

# エントリーポイントにブレークポイントを設定
break start
break main
break uart_pl011_init
break uart_pl011_putchar

# 情報表示
echo \n========================================\n
echo Entry point: start at 0x40200000\n
echo Breakpoints set:\n
echo   - start (entry point)\n
echo   - main (kernel main)\n
echo   - uart_pl011_init (UART initialization)\n
echo   - uart_pl011_putchar (UART output)\n
echo \n========================================\n
echo \nType 'continue' to start execution\n
echo \n

# 現在の状態を表示
info registers
x/10i $pc
EOF

# GDBを起動
$GDB -x /tmp/gdb_commands.txt $KERNEL

