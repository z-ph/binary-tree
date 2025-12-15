@echo off
chcp 65001 >nul
setlocal ENABLEDELAYEDEXPANSION

set BUILD_DIR=%1
if "%BUILD_DIR%"=="" set BUILD_DIR=build
set BINARY=%BUILD_DIR%\avl_tree_tests.exe
set CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -Werror
set INCLUDES=-Iinclude
set SOURCES=src/avl_tree.c tests/avl_tree_tests.c tests/test_framework.c

where gcc >nul 2>nul
if errorlevel 1 (
    echo 未找到 gcc，请先安装 (如 MinGW 或 MSYS2) 并配置好 PATH。
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    if errorlevel 1 (
        echo 无法创建目录 %BUILD_DIR%。
        exit /b 1
    )
)

echo 正在编译测试程序...
gcc %CFLAGS% %INCLUDES% %SOURCES% -o "%BINARY%"
if errorlevel 1 (
    echo 编译失败。
    exit /b 1
)

echo 运行测试...
"%BINARY%"
set EXIT_CODE=%ERRORLEVEL%
if not "%EXIT_CODE%"=="0" (
    echo 测试失败，退出码 %EXIT_CODE%。
) else (
    echo 测试已全部通过。
)

endlocal
exit /b %EXIT_CODE%
