import time
import socket
import urllib.request
import urllib.error
import re
from http.cookiejar import CookieJar
from urllib.parse import quote
from datetime import datetime

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
    """從重新導向URL中提取閘道IP"""
    # 支援動態擷取 HTTP/HTTPS 協議下的 IP 位址
    match = re.search(r"https?://(\d+\.\d+\.\d+\.\d+)", redirect_url)
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
        print(f"{get_timestamp()} 嘗試存取登入頁面...")
    except Exception as e:
        print(f"{get_timestamp()} 初始請求失敗: {e}")
        return

    # 提取重新導向URL
    redirect_url = extract_redirect_url(initial_text)
    if not redirect_url:
        print(f"{get_timestamp()} 無法提取重新導向URL。")
        return
        
    gateway_ip = extract_gateway_ip(redirect_url)
    if not gateway_ip:
        print(f"{get_timestamp()} 無法從重新導向URL提取閘道IP")
        return
        
    try:
        req = urllib.request.Request(redirect_url)
        with opener.open(req, timeout=5) as response:
            login_page_text = response.read().decode('utf-8', errors='ignore')
        if not check_captive_portal_title(login_page_text):
            print(f"{get_timestamp()} 警告：您可能未連線到正確的校園網路。")
            return
        else:
            print(f"{get_timestamp()} 偵測到正確的認證入口頁面")
    except Exception as e:
        print(f"{get_timestamp()} 取得登入頁面失敗: {e}")
        return

    # 提取magic參數
    magic = extract_magic_from_url(redirect_url)
    if not magic:
        print(f"{get_timestamp()} 無法提取magic參數。")
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
            print(f"{get_timestamp()} 自動登入成功!")
        else:
            print(f"{get_timestamp()} 登入請求完成，但狀態可能不成功")
            
    except urllib.error.URLError as e:
        # 如果是 HTTP 錯誤（非 200），但我們仍需要分析內容
        if hasattr(e, 'read'):
            response_text = e.read().decode('utf-8', errors='ignore')
            if "/keepalive?" in response_text.lower():
                print(f"{get_timestamp()} 自動登入成功!")
            else:
                print(f"{get_timestamp()} 登入POST請求失敗(HTTP異常): {e}")
        else:
            print(f"{get_timestamp()} 登入POST請求失敗: {e}")

def main():
    # ASCII Art Banner
    banner = """
   _   _  ____ _   _ _____   ___       _                       _   
  | \\ | |/ ___| | | |_   _| |_ _|_ __ | |_ ___ _ __ _ __   ___| |_ 
  |  \\| | |   | | | | | |    | || '_ \\| __/ _ \\ '__| '_ \\ / _ \\ __|
  | |\\  | |___| |_| | | |    | || | | | ||  __/ |  | | | |  __/ |_ 
  |_| \\_|\\____|\\___/  |_|_  |___|_| |_|\\__\\___|_|  |_| |_|\\___|\\__|
   / \\  _   _| |_ ___   | |    ___   __ _(_)_ __   \\ \\   / /___ \\  
  / _ \\| | | | __/ _ \\  | |   / _ \\ / _` | | '_ \\   \\ \\ / /  __) | 
 / ___ \\ |_| | || (_) | | |__| (_) | (_| | | | | |   \\ V /  / __/  
/_/   \\_\\__,_|\\__\\___/  |_____\\___/ \\__, |_|_| |_|    \\_/  |_____| 
                                    |___/                         
"""
    print(banner)
    print("NCUT 校園網自動登入 V3") # 大版本更新，我認為需要修改版本號
    print("by sangege\n")
    print("Credit: hongfu553, AILIFE-4798, rileychh")
    print("https://github.com/apple050620312/NCUT-Internet-Auto-Login\n")
    
    # 更改帳號密碼區域（請在此修改您的帳號和密碼）
    # 如使用一鍵安裝腳本，請賦予編輯器administrator權限，才能直接修改此區域的內容並儲存
    global account, password
    account = "請替換為您的帳號並儲存（s+您的學號皆小寫）"
    password = "請替換為您的密碼並儲存（身分證字號字母大寫）"
    
    # 防呆機制：檢查是否忘記修改帳號密碼
    if "請替換" in account or "請替換" in password or account == "" or password == "":
        print("\n==============================================")
        print("[錯誤] 您尚未設定帳號密碼！")
        print("請使用記事本打開這支程式，將 account 與 password 變數替換成您的資料。")
        print("程式即將退出...")
        print("==============================================\n")
        time.sleep(5)
        return
    
    # 在啟動時顯示帳號和密碼 (密碼進行星號隱藏保護)
    print("使用的帳號: " + account)
    print("使用的密碼: " + "*" * len(password) + " (為了安全，已隱藏保護)\n\n")

    failed_attempts = 0
    was_offline = False
    
    while True:
        # 第一層：實體斷線防護 (避免沒插線時瘋狂報錯)
        if not is_system_network_connected():
            if not was_offline:
                print(f"{get_timestamp()} 系統未連接網路 (請檢查 Wi-Fi 或網路線)...")
                was_offline = True
            time.sleep(3)
            continue
            
        was_offline = False
        status = check_login_status()
        
        if status == 'ONLINE':
            if failed_attempts > 0:
                print(f"{get_timestamp()} 網路連線正常且已登入!")
                failed_attempts = 0
            time.sleep(5)
            
        elif status == 'NEEDS_LOGIN':
            print(f"{get_timestamp()} 偵測到未登入狀態，立即執行登入...")
            login()
            time.sleep(2)
            
        elif status == 'UNSTABLE':
            failed_attempts += 1
            print(f"{get_timestamp()} 實體有連線，但無法存取外網或不穩定 (第{failed_attempts}次重試)")
            time.sleep(3)

if __name__ == "__main__":
    main()
