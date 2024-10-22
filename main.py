import time
import socket
import requests
# from bs4 import BeautifulSoup
import re
from urllib.parse import quote

# 設定登入資訊
account = "s0X000000@student.ncut.edu.tw"
password = "X000000000"
ip_segment = "180"  # New config option for IP address segment

def check_connection(timeout=1):
    try:
        # 嘗試連接 Google DNS 伺服器 (8.8.8.8), 53 為 DNS 服務的端口
        socket.create_connection(("8.8.8.8", 53), timeout=timeout)
        time.sleep(1)
        return True  # 如果在 timeout 內連接成功，返回 True
    except OSError:
        time.sleep(1)
        return False  # 如果連接失敗或超時，返回 False

def extract_magic_from_url(url):
    match = re.search(r'fgtauth\?([^&]+)', url)
    if match:
        return match.group(1)
    return None

def extract_redirect_url(page_content):
    match = re.search(fr'window\.location="(http://172\.16\.{ip_segment}\.254:1000/fgtauth\?[^\"]+)"', page_content)
    if match:
        return match.group(1)
    return None

def login():
    session = requests.Session()
    
    try:
        initial_response = session.get("http://www.gstatic.com/generate_204")
        print("Trying to access login page...")
        if check_connection():
            print("8.8.8.8 Connected.\n")
    except Exception as e:
        print("Initial request failed:", e)
        if check_connection():
            print("8.8.8.8 Connected.\n")
        return
    
    redirect_url = extract_redirect_url(initial_response.text)
    if redirect_url:
        print("Extracted Redirect URL:", redirect_url)
        if check_connection():
            print("8.8.8.8 Connected.\n")
    else:
        print("Failed to extract redirect URL.")
        if check_connection():
            print("8.8.8.8 Connected.\n")
        return
    
    try:
        login_page = session.get(redirect_url)
    except Exception as e:
        print("Failed to fetch the login page:", e)
        if check_connection():
            print("8.8.8.8 Connected.\n")
        return
    
    # 使用提供的 Full Action URL
    full_action_url = f'http://172.16.{ip_segment}.254:1000/'
    
    # 從 Extracted Redirect URL 提取 magic 參數
    magic = extract_magic_from_url(redirect_url)
    
    if not magic:
        print("Failed to extract magic parameter.")
        if check_connection():
            print("8.8.8.8 Connected.\n")
        return
    else:
        print(f"magic={magic}")
        if check_connection():
            print("8.8.8.8 Connected.\n")

    # 構建 POST 請求的 URL 和資料
    headers = {
        'Content-Type': 'application/x-www-form-urlencoded',
        'Upgrade-Insecure-Requests': '1',
        'Referer': redirect_url,
        'Origin': f'http://172.16.{ip_segment}.254:1000'
    }
    
    login_data = {
        '4Tredir': 'http://www.gstatic.com/generate_204',
        'magic': magic,
        'username': account,
        'password': password
    }
    
    # 將資料轉換為 URL 編碼格式
    encoded_login_data = '&'.join(f'{quote(k)}={quote(v)}' for k, v in login_data.items())
    
    try:
        response = session.post(full_action_url, data=encoded_login_data, headers=headers, timeout=30)
        print("Auto Login Successful")
        if check_connection():
            print("8.8.8.8 Connected.\n")
    except requests.exceptions.RequestException as e:
        pass
        print("Login POST request failed:", e)
        if check_connection():
            print("8.8.8.8 Connected.\n")

def main():
    print("NCUT Internet Auto Login")
    print("by sangege\n")
    if check_connection():
            print("8.8.8.8 Connected.\n")

    failed_attempts = 0  # 用來追蹤連接失敗的次數

    while True:
        if not check_connection():
            failed_attempts += 1
            if failed_attempts >= 2:  # 如果連續兩次連接失敗
                print("8.8.8.8 didn't response last two attempts")
                login()
                failed_attempts = 0  # 重置計數器
            time.sleep(1)  # 增加等待時間以避免頻繁重試
        else:
            failed_attempts = 0  # 成功連接時重置計數器

if __name__ == "__main__":
    main()
