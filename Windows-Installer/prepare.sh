#!/bin/bash
# ==============================================================================
# 快速啟動腳本 - 下載所有必要檔案
# ==============================================================================

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "==============================================="
echo "NCUT 校園網自動登入 - Windows Installer 準備工具"
echo "==============================================="
echo ""

# 檢查是否存在 files 目錄
if [ ! -d "files" ]; then
    mkdir -p files
    echo "[✓] 建立 files/ 目錄"
fi

# 下載 WinSW
WINSW_VERSION="2.12.0"
WINSW_URL="https://github.com/winsw/winsw/releases/download/v${WINSW_VERSION}/WinSW-x64.exe"
WINSW_DEST="files/winsw.exe"

if [ -f "$WINSW_DEST" ]; then
    echo "[✓] WinSW 已存在：$WINSW_DEST"
else
    echo "[→] 下載 WinSW v${WINSW_VERSION}..."
    curl -L -o "$WINSW_DEST" "$WINSW_URL"
    echo "[✓] WinSW 下載完成"
fi

echo ""
echo "==============================================="
echo "準備完成！"
echo "==============================================="
echo ""
echo "下一步："
echo "1. 在 Windows 上安裝 Inno Setup 6"
echo "   下載：https://jrsoftware.org/isdl.php"
echo ""
echo "2. 開啟 NCUT-Auto-Login-Setup.iss"
echo ""
echo "3. 點擊 Build → Compile"
echo ""
echo "4. 輸出的安裝檔位於 Output/ 目錄"
echo ""
echo "==============================================="