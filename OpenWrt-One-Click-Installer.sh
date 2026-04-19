#!/bin/sh
# OpenWrt One-Click Installer for NCUT Auto Login
# Runs interactive setup for pure shell script without Python.

echo "=============================================="
echo " NCUT Auto Login - OpenWrt One-Click Installer "
echo "=============================================="

if [ "$(id -u)" -ne 0 ]; then
    echo "[!] 警告: 請使用 root 權限執行 (OpenWrt 預設即為 root)。"
fi

if ! command -v curl >/dev/null 2>&1; then
    echo "[*] 正在安裝必要套件 (curl)..."
    opkg update
    opkg install curl
    if ! command -v curl >/dev/null 2>&1; then
        echo "[!] 安裝 curl 失敗，請確認路由器是否連上網路！"
        exit 1
    fi
fi

echo ""
echo -n "請輸入您的帳號 (s+您的學號皆小寫): "
read NCUT_ACCOUNT < /dev/tty
echo -n "請輸入您的密碼 (身分證字號字母大寫): "
read NCUT_PASSWORD < /dev/tty
echo ""
echo ""

DIR="/usr/bin/ncut-autologin"
mkdir -p "$DIR"
SCRIPT_PATH="$DIR/ncut_autologin.sh"

echo "[*] 正在下載 OpenWrt 原生腳本..."
SCRIPT_URL="https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/OpenWrt-Native-Login.sh?t=$(date +%s)"
curl -s -o "$SCRIPT_PATH" "$SCRIPT_URL" || {
    echo "[!] 下載腳本失敗！"
    exit 1
}

chmod +x "$SCRIPT_PATH"

# 修正 Windows 換行符號問題 (CRLF -> LF) 避免執行失敗
sed -i 's/\r$//' "$SCRIPT_PATH"

echo "[*] 覆寫帳號密碼設定..."
sed -i "s/^ACCOUNT=\"YOUR_ACCOUNT\"/ACCOUNT=\"$NCUT_ACCOUNT\"/" "$SCRIPT_PATH"
sed -i "s/^PASSWORD=\"YOUR_PASSWORD\"/PASSWORD=\"$NCUT_PASSWORD\"/" "$SCRIPT_PATH"

echo "[*] 建立 systemd/procd 開機服務..."
INIT_FILE="/etc/init.d/ncut_autologin"
cat > "$INIT_FILE" << 'EOF'
#!/bin/sh /etc/rc.common

START=99
USE_PROCD=1
PROG=/usr/bin/ncut-autologin/ncut_autologin.sh

start_service() {
    procd_open_instance
    procd_set_param command /bin/sh "$PROG"
    procd_set_param respawn
    procd_set_param stdout 1
    procd_set_param stderr 1
    procd_close_instance
}
EOF

chmod +x "$INIT_FILE"

echo "[*] 啟動背景服務..."
/etc/init.d/ncut_autologin enable
/etc/init.d/ncut_autologin start

echo "=============================================="
echo " 安裝成功！(Installation Completed!)"
echo " 自動登入服務已經在背景無感運行。"
echo ""
echo " 日誌查詢指令: logread -e ncut-autologin -f"
echo "=============================================="
