import time
import socket
import requests
import re
from urllib.parse import quote
from datetime import datetime

def get_timestamp():
    """獲取當前時間戳記"""
    return datetime.now().strftime("[%Y-%m-%d %H:%M:%S]")

def check_connection(timeout=1):
    """檢查網路連線狀態"""
    try:
        socket.create_connection(("1.1.1.1", 53), timeout=timeout)
        time.sleep(1)
        return True
    except OSError:
        time.sleep(1)
        return False

def extract_magic_from_url(url):
    """從 URL 中提取 magic 參數"""
    match = re.search(r"fgtauth\?([^&]+)", url)
    return match.group(1) if match else None

def extract_redirect_url(page_content):
    """從頁面內容中提取重新導向 URL"""
    match = re.search(
        r'window\.location="(http://\d+\.\d+\.\d+\.\d+:1000/fgtauth\?[^\"]+)"',
        page_content,
    )
    return match.group(1) if match else None

def extract_gateway_ip(redirect_url):
    """從重新導向URL中提取閘道IP"""
    match = re.search(r"http://(\d+\.\d+\.\d+\.\d+):1000", redirect_url)
    if match:
        # 將 IP 的最後一部分替換為 254
        return f"{match.group(1)}"
    return None

def check_captive_portal_title(page_content):
    """檢查 captive portal 頁面標題是否符合"""
    title_pattern = r"勤益科技大學"
    match = re.search(title_pattern, page_content, re.IGNORECASE)
    return match is not None

def login():
    """執行登入操作"""
    session = requests.Session()
    
    # 初始請求取得重新導向
    try:
        initial_response = session.get("http://www.gstatic.com/generate_204", timeout=5)
        print(f"{get_timestamp()} 嘗試存取登入頁面...")
    except Exception as e:
        print(f"{get_timestamp()} 初始請求失敗: {e}")
        return

    # 提取重新導向 URL
    redirect_url = extract_redirect_url(initial_response.text)
    if not redirect_url:
        print(f"{get_timestamp()} 無法提取重新導向 URL。")
        return
        
    print(f"{get_timestamp()} 提取的重新導向 URL: {redirect_url}")

    # 從重新導向 URL 提取閘道 IP（始終為 x.x.x.254）
    gateway_ip = extract_gateway_ip(redirect_url)
    if not gateway_ip:
        print(f"{get_timestamp()} 無法從重新導向 URL 提取閘道 IP")
        return
        
    print(f"{get_timestamp()} 使用閘道 IP: {gateway_ip}")

    # 檢查是否為正確的認證頁面
    try:
        login_page_response = session.get(redirect_url, timeout=5)
        login_page_response.encoding = 'utf-8'  # 確保使用 UTF-8 編碼
        if not check_captive_portal_title(login_page_response.text):
            print(f"{get_timestamp()} Captive portal 標題與預期模式不符合。")
            print(f"{get_timestamp()} 您可能未連線到正確的網路。")
            return
        else:
            print(f"{get_timestamp()} 偵測到正確的認證入口頁面")
    except Exception as e:
        print(f"{get_timestamp()} 取得登入頁面失敗: {e}")
        return

    # 提取 magic 參數
    magic = extract_magic_from_url(redirect_url)
    if not magic:
        print(f"{get_timestamp()} 無法提取 magic 參數。")
        return
        
    print(f"{get_timestamp()} magic 參數: {magic}")

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

    # 發送登入請求
    try:
        response = session.post(
            f"http://{gateway_ip}:1000/",
            data=encoded_login_data,
            headers=headers,
            timeout=30
        )
        
        # 檢查登入是否成功
        if response.status_code == 200 and "/keepalive?" in response.text.lower():
            print(f"{get_timestamp()} 自動登入成功!")
        else:
            print(f"{get_timestamp()} 登入請求完成，但狀態可能不成功")
            
    except requests.exceptions.RequestException as e:
        print(f"{get_timestamp()} 登入 POST 請求失敗: {e}")

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
    print("NCUT 校園網自動登入 v2")
    print("by sangege & AI LIFE\n")
    print("https://github.com/apple050620312/NCUT-Internet-Auto-Login\n")
    
    # 設定登入資訊
    global account, password
    account = "ncut"  # 替換為您的帳號
    password = "ncut"    # 替換為您的密碼
    # 可以使用後門帳號 `ncut/a` 登入，但不保證一定成功
    
    # 在啟動時顯示帳號和密碼
    print("使用的帳號: " + account)
    print("使用的密碼: " + password + "\n\n\n")

    failed_attempts = 1

    while True:
        if not check_connection():
            failed_attempts += 1
            print(f"{get_timestamp()} 連線失敗 (第{failed_attempts}次嘗試)")
            
            if failed_attempts >= 2:
                print(f"{get_timestamp()} 嘗試登入...")
                login()
                failed_attempts = 1
            time.sleep(2)
        else:
            if failed_attempts > 0:
                print(f"{get_timestamp()} 連線已恢復")
                failed_attempts = 0
            time.sleep(5)

if __name__ == "__main__":
    main()
