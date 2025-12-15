#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
BINARY="${BUILD_DIR}/avl_cli"
CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -Werror"
INCLUDES="-Iinclude"
SOURCES="src/avl_tree.c src/main.c"

if ! command -v gcc >/dev/null 2>&1; then
    echo "gcc 未安装，请先安装（如 Xcode Command Line Tools 或 Homebrew gcc）。" >&2
    exit 1
fi

mkdir -p "$BUILD_DIR"

set -x
gcc $CFLAGS $INCLUDES $SOURCES -o "$BINARY"
"$BINARY"
