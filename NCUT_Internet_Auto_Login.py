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

def check_connection(timeout=2):
    """檢查網路是否暢通 (精準檢測 Captive Portal)"""
    try:
        req = urllib.request.Request("http://www.gstatic.com/generate_204")
        with urllib.request.urlopen(req, timeout=timeout) as response:
            # 204 代表沒有被重新導向，真正連到外網
            return response.getcode() == 204
    except Exception:
        return False

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
    """檢查captive portal頁面標題是否符合"""
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
        
    print(f"{get_timestamp()} 提取的重新導向URL: {redirect_url}")

    # 從重新導向URL提取閘道IP（始終為x.x.x.254）
    gateway_ip = extract_gateway_ip(redirect_url)
    if not gateway_ip:
        print(f"{get_timestamp()} 無法從重新導向URL提取閘道IP")
        return
        
    print(f"{get_timestamp()} 使用閘道IP: {gateway_ip}")

    # 檢查是否為正確的認證頁面
    try:
        req = urllib.request.Request(redirect_url)
        with opener.open(req, timeout=5) as response:
            login_page_text = response.read().decode('utf-8', errors='ignore')
        if not check_captive_portal_title(login_page_text):
            print(f"{get_timestamp()} Captive portal標題與預期模式不符合。")
            print(f"{get_timestamp()} 您可能未連線到正確的網路。")
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
        
    print(f"{get_timestamp()} magic參數: {magic}")

    # 準備登入資料
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

    # 編碼登入資料
    encoded_login_data = "&".join(
        f"{quote(k)}={quote(v)}" for k, v in login_data.items()
    )
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
    print("NCUT校園網自動登入V2")
    print("by sangege & AI LIFE\n")
    print("https://github.com/apple050620312/NCUT-Internet-Auto-Login\n")
    
    # 設定登入資訊
    global account, password
    account = "請替換為您的帳號並儲存（s+您的學號皆小寫）"
    password = "請替換為您的密碼並儲存（身分證字號字母大寫）"
    
    # 防呆機制：檢查是否忘記修改帳號密碼
    if "請替換" in account or "請替換" in password or account == "" or password == "":
        print("\n==============================================")
        print("[錯誤] 您尚未設定帳號密碼！")
        print("請使用記事本或編輯器打開這支程式，")
        print("將 account 與 password 變數替換成您的學號與密碼。")
        print("程式即將退出...")
        print("==============================================\n")
        time.sleep(5)
        return
    
    # 在啟動時顯示帳號和密碼 (密碼進行星號隱藏保護)
    print("使用的帳號: " + account)
    print("使用的密碼: " + "*" * len(password) + " (為了安全，已隱藏保護)\n\n\n")

    failed_attempts = 0

    while True:
        if not check_connection(timeout=2):
            failed_attempts += 1
            print(f"{get_timestamp()} 偵測到斷線或未認證 (目前連續 {failed_attempts} 次)")
            
            # 連續偵測失敗 2 次就立刻重連 (比原本 5 次更快)
            if failed_attempts >= 2:
                print(f"{get_timestamp()} 迅速嘗試重新登入...")
                login()
                failed_attempts = 0
                time.sleep(2) # 登入後給一點緩衝時間
            else:
                time.sleep(1) # 第一次失敗後，等 1 秒馬上再測
        else:
            if failed_attempts > 0:
                print(f"{get_timestamp()} 網路已確認暢通！")
                failed_attempts = 0
            time.sleep(2) # 正常狀態下，縮短每 2 秒檢查一次

if __name__ == "__main__":
    main()
