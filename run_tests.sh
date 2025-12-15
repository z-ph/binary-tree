#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
BINARY="${BUILD_DIR}/avl_tree_tests"
CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -Werror"
INCLUDES="-Iinclude"
SOURCES="src/avl_tree.c tests/avl_tree_tests.c tests/test_framework.c"

if ! command -v gcc >/dev/null 2>&1; then
    echo "gcc 未安装，请先安装后再运行此脚本。" >&2
    exit 1
fi

mkdir -p "$BUILD_DIR"

set -x
gcc $CFLAGS $INCLUDES $SOURCES -o "$BINARY"
./"$BINARY"
