#!/bin/sh
# NCUT Internet Auto Login (OpenWrt Native Version)
# Author: sangege (Ported to Shell for OpenWrt)

ACCOUNT="YOUR_ACCOUNT"
PASSWORD="YOUR_PASSWORD"
COOKIE_JAR="/tmp/ncut_cookie.txt"
TMP_HTML="/tmp/ncut_check.html"

log() {
    echo "[$1]"
    # Send to system log
    logger -t "ncut-autologin" "$1"
}

is_system_network_connected() {
    # 僅查詢本地路由表確認是否具備預設閘道 (確認實體連線有取得 IP)
    # 絕對不可使用 ping，因為未登入認證前，防火牆會直接阻擋向外的 ICMP 封包導致誤判。
    if route -n 2>/dev/null | grep -q "^0\.0\.0\.0" || ip route 2>/dev/null | grep -q "^default"; then
        return 0
    else
        return 1
    fi
}

check_login_status() {
    res=$(curl -s -L -w "%{http_code}|%{url_effective}" "http://www.gstatic.com/generate_204" -o "$TMP_HTML")
    http_code=$(echo "$res" | cut -d'|' -f1)
    url_eff=$(echo "$res" | cut -d'|' -f2)
    
    if [ "$http_code" = "204" ]; then
        echo "ONLINE"
        return 0
    fi
    
    if grep -qiE "fgtauth|勤益科技大學" "$TMP_HTML" 2>/dev/null || echo "$url_eff" | grep -qi "fgtauth"; then
        echo "NEEDS_LOGIN"
        return 0
    fi
    
    echo "UNSTABLE"
}

login() {
    rm -f "$COOKIE_JAR"
    
    res=$(curl -s -L -c "$COOKIE_JAR" -w "%{http_code}|%{url_effective}" "http://www.gstatic.com/generate_204" -o "$TMP_HTML")
    url_eff=$(echo "$res" | cut -d'|' -f2)
    
    redirect_url=$(grep -oE "https?://[^'\"]+/fgtauth\?[^'\"]+" "$TMP_HTML" 2>/dev/null | head -n1)
    
    if [ -z "$redirect_url" ] && echo "$url_eff" | grep -qi "fgtauth"; then
        redirect_url="$url_eff"
    fi
    
    if [ -z "$redirect_url" ]; then
        log "登入異常: 無法從頁面解析重新導向網址(Redirect URL)。"
        return 1
    fi
    
    gateway_ip=$(echo "$redirect_url" | grep -oE "https?://[^/:]+" | sed 's|https://||' | sed 's|http://||')
    if [ -z "$gateway_ip" ]; then
        log "登入異常: 無法從重新導向網址解析閘道(Gateway)。"
        return 1
    fi
    
    magic=$(echo "$redirect_url" | grep -oE "fgtauth\?[^&]+" | cut -d'?' -f2)
    if [ -z "$magic" ]; then
        log "登入異常: 無法提取認證 magic 參數。"
        return 1
    fi
    
    log "狀態: 正在向閘道器 $gateway_ip 發送認證請求..."
    
    post_res=$(curl -s -b "$COOKIE_JAR" \
        -w "%{http_code}" \
        -H "Content-Type: application/x-www-form-urlencoded" \
        -H "Upgrade-Insecure-Requests: 1" \
        -H "Referer: $redirect_url" \
        -H "Origin: http://$gateway_ip:1000" \
        --data-urlencode "4Tredir=http://www.gstatic.com/generate_204" \
        --data-urlencode "magic=$magic" \
        --data-urlencode "username=$ACCOUNT" \
        --data-urlencode "password=$PASSWORD" \
        "http://$gateway_ip:1000/" -o "$TMP_HTML")
        
    if grep -qFi "/keepalive?" "$TMP_HTML" 2>/dev/null; then
        log "登入成功: 已成功完成校園網路認證！"
    else
        log "登入異常: 登入請求完成，但未偵測到成功標記。HTTP Code: $post_res"
    fi
}

# --- Main Loop ---
log "NCUT Auto Login (OpenWrt Native) 啟動"

if ! command -v curl >/dev/null 2>&1; then
    log "錯誤: 系統缺少 curl 指令，腳本無法運行。請執行 opkg install curl。"
    exit 1
fi

last_state="INITIAL"

while true; do
    if ! is_system_network_connected; then
        if [ "$last_state" != "SYSTEM_OFFLINE" ]; then
            log "等待: 設備尚未建立路由到外部網路，等待路由恢復..."
            last_state="SYSTEM_OFFLINE"
        fi
        sleep 3
        continue
    fi
    
    status=$(check_login_status)
    
    if [ "$status" = "ONLINE" ]; then
        if [ "$last_state" != "ONLINE" ]; then
            log "連線正常: 已經在網路登入狀態！"
            last_state="ONLINE"
        fi
        sleep 5
        
    elif [ "$status" = "NEEDS_LOGIN" ]; then
        if [ "$last_state" != "NEEDS_LOGIN" ]; then
            log "未登入狀態: 準備執行自動登入程序..."
            last_state="NEEDS_LOGIN"
        fi
        login
        sleep 2
        
    elif [ "$status" = "UNSTABLE" ]; then
        if [ "$last_state" != "UNSTABLE" ]; then
            log "警告: 已連接但無法存取外網或找到認證頁。這通常代表網路異常。"
            last_state="UNSTABLE"
        fi
        sleep 3
    else
        sleep 3
    fi
done
