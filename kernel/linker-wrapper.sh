#!/bin/bash
# Linker wrapper to force static EXEC binary generation
exec rust-lld "$@" -static --no-pie -znorelro --no-dynamic-linker
