#!/bin/bash
# Linker wrapper to force static EXEC binary generation
SYSROOT=$(rustc --print sysroot)
RUST_LLD="$SYSROOT/lib/rustlib/x86_64-unknown-linux-gnu/bin/rust-lld"
exec "$RUST_LLD" "$@" -static --no-pie -znorelro --no-dynamic-linker
