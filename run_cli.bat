@echo off
chcp 65001 >nul
setlocal ENABLEDELAYEDEXPANSION

set BUILD_DIR=%1
if "%BUILD_DIR%"=="" set BUILD_DIR=build
set BINARY=%BUILD_DIR%\avl_cli.exe
set CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -Werror
set INCLUDES=-Iinclude
set SOURCES=src/avl_tree.c src/main.c

where gcc >nul 2>nul
if errorlevel 1 (
    echo 未找到 gcc，請先安裝 (如 MinGW-w64 或 MSYS2) 並配置 PATH。
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    if errorlevel 1 (
        echo 無法創建目錄 %BUILD_DIR%。
        exit /b 1
    )
)

echo 正在編譯 CLI...
gcc %CFLAGS% %INCLUDES% %SOURCES% -o "%BINARY%"
if errorlevel 1 (
    echo 編譯失敗。
    exit /b 1
)

echo 啟動 AVL CLI...
"%BINARY%"
set EXIT_CODE=%ERRORLEVEL%

endlocal
exit /b %EXIT_CODE%
