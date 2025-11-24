#!/bin/bash
# QEMUシリアル出力確認スクリプト

echo "=========================================="
echo "QEMUシリアル出力確認"
echo "=========================================="
echo ""

echo "1. QEMUの起動オプションを確認"
echo "   -serial stdio: シリアル出力を標準出力に接続"
echo "   -nographic: グラフィック出力を無効化"
echo ""

echo "2. QEMUを起動したターミナルで確認"
echo "   シリアル出力は、QEMUを起動したターミナルに直接表示されます"
echo ""

echo "3. テスト方法"
echo "   a) QEMUを起動: ./run-tkernel-qemu.sh"
echo "   b) 別のターミナルでGDBで接続"
echo "   c) GDBでUARTDRに手動でデータを書き込む"
echo "   d) QEMUのターミナルに文字が表示されるか確認"
echo ""

echo "4. 手動テストコマンド（GDBで実行）"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x48  # 'H'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x65  # 'e'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x6c  # 'l'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x6c  # 'l'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x6f  # 'o'"
echo "   (gdb) set *(volatile unsigned int*)0x09000000 = 0x0a  # '\n'"
echo ""

echo "=========================================="
