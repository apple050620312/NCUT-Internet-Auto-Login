import time
import socket
import requests
import re
from urllib.parse import quote
import subprocess

def get_ip_segment():
    try:
        # Get all IP configurations using ipconfig command with 'cp950' encoding for Traditional Chinese Windows
        output = subprocess.check_output("ipconfig", shell=True).decode('cp950')
        
        # Look for IPv4 addresses matching our pattern (support both English and Chinese Windows)
        ip_pattern = r"(?:IPv4.*?Address|IPv4.*?位址)[.\s]*: 172\.16\.(\d{1,3})\.\d{1,3}"
        match = re.search(ip_pattern, output)
        
        if match:
            # Return the third octet (xxx from 172.16.xxx.yyy)
            return match.group(1)
        else:
            # Default fallback if no matching IP is found
            print("Warning: Could not detect IP segment automatically. Using default value.")
            return segment
    except Exception as e:
        print(f"Error detecting IP segment: {e}")
        return segment

def check_connection(timeout=1):
    try:
        socket.create_connection(("8.8.8.8", 53), timeout=timeout)
        time.sleep(1)
        return True
    except OSError:
        time.sleep(1)
        return False

def extract_magic_from_url(url):
    match = re.search(r'fgtauth\?([^&]+)', url)
    if match:
        return match.group(1)
    return None

def extract_redirect_url(page_content, ip_segment):
    match = re.search(fr'window\.location="(http://172\.16\.{ip_segment}\.254:1000/fgtauth\?[^\"]+)"', page_content)
    if match:
        return match.group(1)
    return None

def login(ip_segment):
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
    
    redirect_url = extract_redirect_url(initial_response.text, ip_segment)
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
    
    full_action_url = f'http://172.16.{ip_segment}.254:1000/'
    
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
    
    encoded_login_data = '&'.join(f'{quote(k)}={quote(v)}' for k, v in login_data.items())
    
    try:
        response = session.post(full_action_url, data=encoded_login_data, headers=headers, timeout=30)
        print("Auto Login Successful")
        if check_connection():
            print("8.8.8.8 Connected.\n")
    except requests.exceptions.RequestException as e:
        print("Login POST request failed:", e)
        if check_connection():
            print("8.8.8.8 Connected.\n")

def main():
    print("NCUT Internet Auto Login")
    print("by sangege\n")

    # Get IP segment at startup
    ip_segment = get_ip_segment()
    print(f"Detected IP segment: {ip_segment}\n")

    if check_connection():
        print("8.8.8.8 Connected.\n")

    failed_attempts = 0

    while True:
        if not check_connection():
            failed_attempts += 1
            if failed_attempts >= 2:
                print("8.8.8.8 didn't response last two attempts")
                login(ip_segment)
                failed_attempts = 0
            time.sleep(1)
        else:
            failed_attempts = 0

if __name__ == "__main__":
    # 設定登入資訊
    account = "ncut"
    password = "a"
    segment = "100"
    main()
