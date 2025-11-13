#!/bin/bash
# QEMUのUART設定を確認するスクリプト

echo "=========================================="
echo "QEMU UART設定確認"
echo "=========================================="
echo ""

echo "1. QEMUの起動オプションを確認"
echo "   -serial stdio: シリアル出力を標準出力に接続"
echo "   -nographic: グラフィック出力を無効化"
echo ""

echo "2. QEMUのUARTデバイスアドレス"
echo "   PL011 UART base address: 0x09000000"
echo ""

echo "3. 確認方法"
echo "   a) QEMUを起動: ./run-tkernel-qemu.sh"
echo "   b) 別のターミナルでGDBで接続"
echo "   c) UARTDRに直接書き込んでテスト"
echo ""

echo "4. テストコマンド（GDBで実行）"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x48  # 'H'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x65  # 'e'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x6c  # 'l'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x6c  # 'l'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x6f  # 'o'"
echo ""

echo "=========================================="
