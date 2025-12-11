#!/bin/bash
# Linker wrapper to force static EXEC binary generation
echo "[LINKER-WRAPPER] Called with args: $@" >&2
SYSROOT=$(rustc --print sysroot)
RUST_LLD="$SYSROOT/lib/rustlib/x86_64-unknown-linux-gnu/bin/rust-lld"
echo "[LINKER-WRAPPER] Using rust-lld: $RUST_LLD" >&2
echo "[LINKER-WRAPPER] Adding flags: -static --no-pie -znorelro --no-dynamic-linker" >&2
exec "$RUST_LLD" "$@" -static --no-pie -znorelro --no-dynamic-linker
