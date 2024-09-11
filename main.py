import os
import time
import requests
from bs4 import BeautifulSoup
import re
from urllib.parse import urljoin, parse_qs, quote

# 設定登入資訊
account = "s0X000000@student.ncut.edu.tw"
password = "X000000000"

def check_connection():
    response = os.system("ping -n 1 8.8.8.8")
    print("Ping result:", response)
    return response == 0

def extract_magic_from_url(url):
    match = re.search(r'fgtauth\?([^&]+)', url)
    if match:
        return match.group(1)
    return None

def extract_redirect_url(page_content):
    match = re.search(r'window\.location="(http://172\.16\.180\.254:1000/fgtauth\?[^\"]+)"', page_content)
    if match:
        return match.group(1)
    return None

def login():
    session = requests.Session()
    
    try:
        initial_response = session.get("http://www.msftconnecttest.com/redirect")
        print("Initial Request URL:", initial_response.url)
        print("Initial Request Status Code:", initial_response.status_code)
        print("Initial Page Content Preview:", initial_response.text[:500])
    except Exception as e:
        print("Initial request failed:", e)
        return
    
    redirect_url = extract_redirect_url(initial_response.text)
    if redirect_url:
        print("Extracted Redirect URL:", redirect_url)
    else:
        print("Failed to extract redirect URL.")
        return
    
    try:
        login_page = session.get(redirect_url)
        print("Final Redirected URL:", login_page.url)
        print("Login Page Content Preview:", login_page.text[:500])
    except Exception as e:
        print("Failed to fetch the login page:", e)
        return
    
    soup = BeautifulSoup(login_page.text, 'html.parser')
    form = soup.find('form')
    if form is None:
        print("Form not found, please check the page structure.")
        return
    
    action_url = form.get('action', '/')
    print("Form Action URL:", action_url)
    
    # 使用提供的 Full Action URL
    full_action_url = 'http://172.16.180.254:1000/'
    
    # 從 Extracted Redirect URL 提取 magic 參數
    magic = extract_magic_from_url(redirect_url)
    
    if not magic:
        print("Failed to extract magic parameter.")
        return

    # 構建 POST 請求的 URL 和資料
    headers = {
        'Content-Type': 'application/x-www-form-urlencoded',
        'Upgrade-Insecure-Requests': '1',
        'Referer': redirect_url,
        'Origin': 'http://172.16.180.254:1000'
    }
    
    login_data = {
        '4Tredir': 'http://www.msftconnecttest.com/redirect',
        'magic': magic,
        'username': account,
        'password': password
    }
    
    # 將資料轉換為 URL 編碼格式
    encoded_login_data = '&'.join(f'{quote(k)}={quote(v)}' for k, v in login_data.items())
    
    # 打印出 POST 請求的詳細資訊
    print("\nPOST Request Details:")
    print("URL:", full_action_url)
    print("Headers:", headers)
    print("Body:", encoded_login_data)
    
    try:
        response = session.post(full_action_url, data=encoded_login_data, headers=headers, timeout=30)
        print("\nLogin POST Request URL:", response.url)
        print("Login POST Request Status Code:", response.status_code)
        print("Login POST Response Content Preview:", response.text[:500])
    except requests.exceptions.RequestException as e:
        print("Login POST request failed:", e)

def main():
    while True:
        if not check_connection():
            print("Disconnected, trying to login...")
            login()
            time.sleep(10)  # 增加等待時間以避免頻繁重試
        else:
            print("Connected.")
        time.sleep(1)

if __name__ == "__main__":
    main()
