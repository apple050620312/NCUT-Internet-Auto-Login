#!/bin/bash

# ==============================================================================
# NCUT 校網自動登入 Mac應用程式版v3.0 by Ruan
# ==============================================================================

# 定義檔案路徑
INSTALL_DIR="$HOME/.ncut_autologin"
PYTHON_FILE="$INSTALL_DIR/ncut_login.py"
PLIST_FILE="$HOME/Library/LaunchAgents/com.user.ncutlogin.plist"
COMMAND_FILE="$HOME/Desktop/NCUT校網自動登入.command"

# ==============================================================================
# 步驟 0. 主選單：詢問使用者要安裝還是解除安裝
# ==============================================================================
ACTION=$(osascript <<EOF
tell application "System Events"
    activate
    try
        set result to button returned of (display dialog "歡迎使用 NCUT 校網自動登入工具Mac應用程式版 v3.0！\n\n請問您想要執行什麼操作？" buttons {"取消", "解除安裝", "安裝 / 更改設定（包含更改帳密）"} default button "安裝 / 更改設定（包含更改帳密）" with title "主選單")
        return result
    on error
        return "取消"
    end try
end tell
EOF
)

# 如果按下取消，直接退出程式
if [ "$ACTION" = "取消" ] || [ -z "$ACTION" ]; then exit 0; fi

# ==============================================================================
# 分支 A：解除安裝流程
# ==============================================================================
if [ "$ACTION" = "解除安裝" ]; then
    # 執行大掃除
    launchctl unload "$PLIST_FILE" 2>/dev/null
    rm "$PLIST_FILE" 2>/dev/null
    rm -rf "$INSTALL_DIR" 2>/dev/null
    rm "$COMMAND_FILE" 2>/dev/null
    rm /tmp/ncut_login.log /tmp/ncut_login.err 2>/dev/null
    osascript -e 'tell application "System Events" to delete (every login item whose name is "NCUT校網自動登入.command")' 2>/dev/null
    
    # 彈出解除安裝成功通知
    osascript <<EOF
tell application "System Events"
    activate
    display dialog "🗑️ 解除安裝完成！\n\n系統內部的所有校網登入設定與隱藏檔案已經徹底清除乾淨。\n您現在可以將這個安裝程式丟入垃圾桶了 👋" buttons {"確定"} default button "確定" with title "解除安裝成功"
end tell
EOF
    exit 0
fi

# ==============================================================================
# 分支 B：安裝流程
# ==============================================================================
mkdir -p "$INSTALL_DIR"

# 1. 詢問帳號 
ACCOUNT=$(osascript <<EOF
tell application "System Events"
    activate
    try
        set result to text returned of (display dialog "【步驟 1/3】\n請輸入您的帳號（s+學號）：" default answer "" with title "安裝精靈")
        return result
    on error
        return ""
    end try
end tell
EOF
)
if [ -z "$ACCOUNT" ]; then exit 0; fi

# 2. 詢問密碼
PASSWORD=$(osascript <<EOF
tell application "System Events"
    activate
    try
        set result to text returned of (display dialog "【步驟 2/3】\n請輸入您的密碼（身分證字號大寫）：" default answer "" with hidden answer with title "安裝精靈")
        return result
    on error
        return ""
    end try
end tell
EOF
)
if [ -z "$PASSWORD" ]; then exit 0; fi

# 3. 讓使用者選擇執行模式
MODE=$(osascript <<EOF
tell application "System Events"
    activate
    try
        set result to button returned of (display dialog "【步驟 3/3】\n請問您希望程式開機時如何執行？\n\n【隱藏背景執行】：完全沒有畫面，默默幫你登入（推薦）。\n【顯示黑色視窗】：跳出終端機畫面，讓你看著它跑。" buttons {"取消", "顯示黑色視窗", "隱藏背景執行"} default button "隱藏背景執行" with title "選擇執行模式")
        return result
    on error
        return "取消"
    end try
end tell
EOF
)
if [ "$MODE" = "取消" ] || [ -z "$MODE" ]; then exit 0; fi

# 4. 寫入 Python 主程式
cat << 'EOF' > "$PYTHON_FILE"
# ==============================================================================
# 👇 NCUT_Internet_Auto_Login.py Python 程式碼👇
import time
import socket
import urllib.request
import urllib.error
import re
from http.cookiejar import CookieJar
from urllib.parse import quote
from datetime import datetime

# ==============================================================================
# 【帳號密碼設定區】 - 請在這裡修改您的帳號和密碼！
# ==============================================================================
# 備註：如使用一鍵安裝腳本，請賦予編輯器 administrator 權限，才能直接修改此處並儲存。
account = "請替換為您的帳號並儲存（s+您的學號皆小寫）"
password = "請替換為您的密碼並儲存（身分證字號字母大寫）"
# ==============================================================================

def get_timestamp():
    """獲取當前時間戳記"""
    return datetime.now().strftime("[%Y-%m-%d %H:%M:%S]")

# 第一層實體網路偵測 (UDP查表法)
def is_system_network_connected():
    """瞬間檢查系統實體網路狀態，0毫秒延遲"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0] != "127.0.0.1"
    except Exception:
        return False

# 第二層登入狀態偵測 (改寫為 urllib 版本)

def check_login_status(max_retries=3):
    """
    檢查網路與登入狀態，區分「未登入」與「真斷線」
    回傳值: 'ONLINE', 'NEEDS_LOGIN', 'UNSTABLE'
    """
    timeout_count = 0
    for attempt in range(max_retries):
        try:
            req = urllib.request.Request("http://www.gstatic.com/generate_204")
            with urllib.request.urlopen(req, timeout=3) as response:
                if response.getcode() == 204:
                    return 'ONLINE'
                
                # 讀取網頁內容，檢查是否包含 Fortinet 攔截特徵
                html = response.read().decode('utf-8', errors='ignore')
                if "fgtauth" in html or "勤益科技大學" in html or "fgtauth" in response.geturl():
                    return 'NEEDS_LOGIN'
                timeout_count += 1
                
        except Exception:
            timeout_count += 1
            
        if timeout_count < max_retries:
            time.sleep(1.5)
            
    return 'UNSTABLE'

def extract_magic_from_url(url):
    """從URL中提取magic參數"""
    match = re.search(r"fgtauth\?([^&]+)", url)
    return match.group(1) if match else None

def extract_redirect_url(page_content):
    """從頁面內容中提取重新導向URL"""
    match = re.search(
        r'window\.location\s*=\s*["\'](https?://[^"\']+/fgtauth\?[^"\']+)["\']',
        page_content,
    )
    return match.group(1) if match else None

def extract_gateway_ip(redirect_url):
    """從重新導向URL中提取閘道 IP 或主機名稱"""
    # 支援動態擷取 HTTP/HTTPS 協議下的 IP 位址或網域 (非 / 及 : 字元)
    match = re.search(r"https?://([^/:]+)", redirect_url)
    if match:
        return match.group(1)
    return None

def check_captive_portal_title(page_content):
    title_pattern = r"勤益科技大學"
    match = re.search(title_pattern, page_content, re.IGNORECASE)
    return match is not None

def login():
    """執行登入操作"""
    # 建立支援 Cookie 的 Opener 來模擬 Session
    cookie_jar = CookieJar()
    opener = urllib.request.build_opener(urllib.request.HTTPCookieProcessor(cookie_jar))
    
    # 初始請求取得重新導向
    try:
        req = urllib.request.Request("http://www.gstatic.com/generate_204")
        with opener.open(req, timeout=5) as response:
            initial_text = response.read().decode('utf-8', errors='ignore')
            initial_url = response.geturl()
    except Exception as e:
        print(f"{get_timestamp()} [登入異常] 初始請求失敗無法取得重新導向: {e}")
        return

    # 提取重新導向URL (優先從網頁內容抓取 JS 跳轉)
    redirect_url = extract_redirect_url(initial_text)
    
    # [額外容錯] 若 Fortinet 直接給了 302 HTTP 跳轉，擷取最終目標網址
    if not redirect_url and "fgtauth" in initial_url:
        redirect_url = initial_url
        
    if not redirect_url:
        print(f"{get_timestamp()} [登入異常] 無法從頁面解析重新導向網址(Redirect URL)。")
        return
        
    gateway_ip = extract_gateway_ip(redirect_url)
    if not gateway_ip:
        print(f"{get_timestamp()} [登入異常] 無法從重新導向網址解析閘道IP(Gateway IP)。")
        return
        
    try:
        req = urllib.request.Request(redirect_url)
        with opener.open(req, timeout=5) as response:
            login_page_text = response.read().decode('utf-8', errors='ignore')
        if not check_captive_portal_title(login_page_text):
            print(f"{get_timestamp()} [警告] 網頁標題不符，可能未連線到勤益校園網路。")
            return
    except Exception as e:
        print(f"{get_timestamp()} [登入異常] 取得登入頁面失敗: {e}")
        return

    # 提取magic參數
    magic = extract_magic_from_url(redirect_url)
    if not magic:
        print(f"{get_timestamp()} [登入異常] 無法提取認證 magic 參數。")
        return

    headers = {
        "Content-Type": "application/x-www-form-urlencoded",
        "Upgrade-Insecure-Requests": "1",
        "Referer": redirect_url,
        "Origin": f"http://{gateway_ip}:1000",
    }

    login_data = {
        "4Tredir": "http://www.gstatic.com/generate_204",
        "magic": magic,
        "username": account,
        "password": password,
    }

    encoded_login_data = "&".join(f"{quote(k)}={quote(v)}" for k, v in login_data.items())
    data_bytes = encoded_login_data.encode('utf-8')

    # 發送登入請求
    try:
        req = urllib.request.Request(
            f"http://{gateway_ip}:1000/",
            data=data_bytes,
            headers=headers,
            method='POST'
        )
        with opener.open(req, timeout=30) as response:
            status_code = response.getcode()
            response_text = response.read().decode('utf-8', errors='ignore')
        
        # 檢查登入是否成功
        if status_code == 200 and "/keepalive?" in response_text.lower():
            print(f"{get_timestamp()} [登入成功] 已成功完成校園網路認證！")
        else:
            print(f"{get_timestamp()} [登入異常] 登入請求完成，但未偵測到成功標記。")
            
    except urllib.error.URLError as e:
        # 如果是 HTTP 錯誤（非 200），但我們仍需要分析內容
        if hasattr(e, 'read'):
            response_text = e.read().decode('utf-8', errors='ignore')
            if "/keepalive?" in response_text.lower():
                print(f"{get_timestamp()} [登入成功] 已成功完成校園網路認證！(帶有 HTTP 異常狀態)")
            else:
                print(f"{get_timestamp()} [登入失敗] POST 請求異常且無成功標記: {e}")
        else:
            print(f"{get_timestamp()} [登入失敗] POST 請求異常: {e}")

def main():
    # ASCII Art Banner
    banner = """
 _   _  _____ _    _ _______   _____       _                       _   
| \ | |/ ____| |  | |__   __| |_   _|     | |                     | |  
|  \| | |    | |  | |  | |      | |  _ __ | |_ ___ _ __ _ __   ___| |_ 
| . ` | |    | |  | |  | |      | | | '_ \| __/ _ \ '__| '_ \ / _ \ __|
| |\  | |____| |__| |  | |     _| |_| | | | ||  __/ |  | | | |  __/ |_ 
|_| \_|\_____|\____/   |_|    |_____|_| |_|\__\___|_|  |_| |_|\___|\__|

               _          _                 _              ____  
    /\        | |        | |               (_)            |___ \ 
   /  \  _   _| |_ ___   | |     ___   __ _ _ _ __   __   ____) |
  / /\ \| | | | __/ _ \  | |    / _ \ / _` | | '_ \  \ \ / /__ < 
 / ____ \ |_| | || (_) | | |___| (_) | (_| | | | | |  \ V /___) |
/_/    \_\__,_|\__\___/  |______\___/ \__, |_|_| |_|   \_/|____/ 
                                       __/ |                     
                                      |___/                      
"""
    print(banner)
    print("NCUT 校園網自動登入 V3") # 大版本更新，我認為需要修改版本號
    print("by sangege\n")
    print("Credit: hongfu553, AILIFE-4798, rileychh")
    print("https://github.com/apple050620312/NCUT-Internet-Auto-Login\n")
    
    # 首次執行初始化與自我覆寫設定
    global account, password
    if "請替換" in account or "請替換" in password or account == "" or password == "":
        import os
        import sys
        
        script_path = os.path.abspath(__file__)
        
        # 測試是否具有寫入權限 (若無權限則嘗試在 Windows 提權)
        try:
            with open(script_path, 'a', encoding='utf-8'): 
                pass
        except PermissionError:
            print("\n==============================================")
            if os.name == 'nt':
                print("【權限不足】偵測到腳本位於保護目錄 (如系統 Startup 資料夾)")
                print("需要管理員權限來儲存您的設定。請在稍後的彈出視窗點擊「是」。")
                print("==============================================")
                time.sleep(2)
                import ctypes
                try:
                    # 使用 runas 重新以管理員權限啟動這支 Python 腳本
                    ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, f'"{script_path}"', None, 1)
                except Exception as e:
                    print(f"\n[錯誤] 提權失敗: {e}")
                    time.sleep(5)
                # 結束當前未提權的程序
                sys.exit(0)
            else:
                print("【權限不足】無法修改腳本本身以儲存設定。")
                print(f"請給予檔案寫入權限 (例如: chmod +w {script_path})")
                print("==============================================")
                time.sleep(5)
                sys.exit(1)

        print("\n==============================================")
        print("【首次設定】偵測到首次執行，請輸入後腳本將自動保存設定。")
        print("==============================================")
        new_account = input("請輸入您的帳號（s+您的學號皆小寫）: ").strip()
        new_password = input("請輸入您的密碼（身分證字號字母大寫）: ").strip()

        if not new_account or not new_password:
            print("[錯誤] 帳號或密碼不能為空，程式即將退出...")
            time.sleep(5)
            sys.exit(1)
            
        try:
            with open(script_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
                
            lines_to_remove = []
            for i in range(len(lines)):
                # 1. 使用 re.sub 替換設定好的帳號和密碼
                if 'account = "請替換' in lines[i]:
                    lines[i] = re.sub(r'account\s*=\s*".*"', f'account = "{new_account}"', lines[i])
                elif 'password = "請替換' in lines[i]:
                    lines[i] = re.sub(r'password\s*=\s*".*"', f'password = "{new_password}"', lines[i])
                
                # 幫忙記錄要刪除的無用提示備註
                elif "請在這裡修改您的帳號和密碼" in lines[i] or "備註：如使用一鍵安裝腳本" in lines[i]:
                    lines_to_remove.append(lines[i])
                    
            # 2. 使用 list.remove 刪除不需要的文字行
            for target in lines_to_remove:
                if target in lines:
                    lines.remove(target)
                    
            with open(script_path, 'w', encoding='utf-8') as f:
                f.writelines(lines)
                
            print("\n[成功] 帳號密碼已保存至腳本！完成自我覆寫。")
            account = new_account
            password = new_password
        except Exception as e:
            print(f"\n[錯誤] 無法覆寫腳本，請確認檔案權限 ({e})")
            return
    
    # 在啟動時顯示帳號和密碼 (密碼進行星號隱藏保護)
    print("使用的帳號: " + account)
    print("使用的密碼: " + "*" * len(password) + " (為了安全，已隱藏保護)\n\n")

    last_state = 'INITIAL'
    
    while True:
        # 第一層：實體斷線防護 (避免沒插線時瘋狂報錯)
        if not is_system_network_connected():
            if last_state != 'SYSTEM_OFFLINE':
                print(f"{get_timestamp()} [用戶問題] 設備未連接網路，請檢查 Wi-Fi 或網路線是否已接上。")
                last_state = 'SYSTEM_OFFLINE'
            time.sleep(3)
            continue
            
        status = check_login_status()
        
        if status == 'ONLINE':
            if last_state != 'ONLINE':
                if last_state == 'INITIAL':
                    print(f"{get_timestamp()} [狀態] 網路連線正常且已登入！")
                else:
                    print(f"{get_timestamp()} [網路恢復] 網路連線已恢復正常且已登入！")
                last_state = 'ONLINE'
            time.sleep(5)
            
        elif status == 'NEEDS_LOGIN':
            if last_state != 'NEEDS_LOGIN':
                print(f"{get_timestamp()} [狀態變更] 偵測到未登入狀態或授權過期，準備執行自動登入程序...")
                last_state = 'NEEDS_LOGIN'
            login()
            time.sleep(2)
            
        elif status == 'UNSTABLE':
            if last_state != 'UNSTABLE':
                print(f"{get_timestamp()} [學校網路問題] 已連接到網路，但無法存取外網且無認證頁面。這通常代表學校網路異常或是設備故障。")
                last_state = 'UNSTABLE'
            time.sleep(3)

if __name__ == "__main__":
    main()

# 👆NCUT_Internet_Auto_Login.py Python 程式碼 👆
# ==============================================================================
EOF

# 5. 將使用者輸入的帳號密碼自動替換進 Python 腳本中
sed -i '' "s/account *= *\".*\"/account = \"$ACCOUNT\"/g" "$PYTHON_FILE"
sed -i '' "s/password *= *\".*\"/password = \"$PASSWORD\"/g" "$PYTHON_FILE"

# 6. 執行切換前的大掃除 
launchctl unload "$PLIST_FILE" 2>/dev/null
rm "$PLIST_FILE" 2>/dev/null
rm "$COMMAND_FILE" 2>/dev/null
osascript -e 'tell application "System Events" to delete (every login item whose name is "NCUT校網自動登入.command")' 2>/dev/null

# 7. 依據使用者的選擇進行設定
if [ "$MODE" = "隱藏背景執行" ]; then

    # 建立背景執行的服務設定檔
    cat << EOF > "$PLIST_FILE"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.user.ncutlogin</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/bin/python3</string>
        <string>$PYTHON_FILE</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
    <key>StandardOutPath</key>
    <string>/tmp/ncut_login.log</string>
    <key>StandardErrorPath</key>
    <string>/tmp/ncut_login.err</string>
</dict>
</plist>
EOF
    # 啟動背景服務
    launchctl load "$PLIST_FILE"
    
    # 顯示成功畫面
    osascript <<EOF
tell application "System Events"
    activate
    display dialog "🎉 安裝與設定已完全自動搞定！\n\n您選擇的是【隱藏背景執行】，程式現在已經在背景默默運作。" buttons {"太棒了！"} default button "太棒了！" with title "安裝成功"
end tell
EOF

elif [ "$MODE" = "顯示黑色視窗" ]; then

    # 在桌面建立可執行的捷徑
    echo "#!/bin/bash" > "$COMMAND_FILE"
    echo "clear" >> "$COMMAND_FILE"
    echo "/usr/bin/python3 \"$PYTHON_FILE\"" >> "$COMMAND_FILE"
    chmod +x "$COMMAND_FILE"

    # 自動加入 Mac 的開機登入項目
    osascript -e "tell application \"System Events\" to make login item at end with properties {path:\"$COMMAND_FILE\", hidden:false}" 2>/dev/null

    # 顯示成功畫面
    osascript <<EOF
tell application "System Events"
    activate
    display dialog "🎉 安裝與設定已完全自動搞定！\n\n您選擇的是【顯示黑色視窗】，已經在您的【桌面】建立了一個啟動捷徑，並且已加入開機自動執行清單！\n\n現在請到桌面點擊「NCUT校網自動登入.command」來開啟程式吧！\n\n⚠️請注意如關閉黑色視窗程式將不會背景運行⚠️" buttons {"太棒了！"} default button "太棒了！" with title "安裝成功"
end tell
EOF

fi
