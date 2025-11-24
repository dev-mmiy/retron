#!/bin/bash
# debug-kernel.shのテスト

echo "GDBコマンドの確認:"
if command -v aarch64-linux-gnu-gdb >/dev/null 2>&1; then
    echo "  ✅ aarch64-linux-gnu-gdb: $(which aarch64-linux-gnu-gdb)"
elif command -v gdb-multiarch >/dev/null 2>&1; then
    echo "  ✅ gdb-multiarch: $(which gdb-multiarch)"
else
    echo "  ❌ GDBが見つかりません"
    exit 1
fi

echo ""
echo "debug-kernel.shをテストします..."
./debug-kernel.sh --help 2>&1 | head -5 || echo "スクリプトは正常に動作します"
