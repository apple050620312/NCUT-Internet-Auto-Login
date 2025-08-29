import time
import socket
import requests
import re
from urllib.parse import quote

def check_connection(timeout=1):
    """检查网络连接状态"""
    try:
        socket.create_connection(("1.1.1.1", 53), timeout=timeout)
        time.sleep(1)
        return True
    except OSError:
        time.sleep(1)
        return False

def extract_magic_from_url(url):
    """从URL中提取magic参数"""
    match = re.search(r"fgtauth\?([^&]+)", url)
    return match.group(1) if match else None

def extract_redirect_url(page_content):
    """从页面内容中提取重定向URL"""
    match = re.search(
        r'window\.location="(http://\d+\.\d+\.\d+\.\d+:1000/fgtauth\?[^\"]+)"',
        page_content,
    )
    return match.group(1) if match else None

def extract_gateway_ip(redirect_url):
    """从重定向URL中提取网关IP"""
    match = re.search(r"http://(\d+\.\d+\.\d+\.\d+):1000", redirect_url)
    if match:
        # 将IP的最后一部分替换为254
        return f"{match.group(1)}"
    return None

def check_captive_portal_title(page_content):
    """检查captive portal页面标题是否匹配"""
    title_pattern = r"勤益科技大學"
    match = re.search(title_pattern, page_content, re.IGNORECASE)
    return match is not None

def login():
    """执行登录操作"""
    session = requests.Session()
    
    # 初始请求获取重定向
    try:
        initial_response = session.get("http://www.gstatic.com/generate_204", timeout=5)
        print("尝试访问登录页面...")
    except Exception as e:
        print(f"初始请求失败: {e}")
        return

    # 提取重定向URL
    redirect_url = extract_redirect_url(initial_response.text)
    if not redirect_url:
        print("无法提取重定向URL。")
        return
        
    print(f"提取的重定向URL: {redirect_url}")

    # 从重定向URL提取网关IP（始终为x.x.x.254）
    gateway_ip = extract_gateway_ip(redirect_url)
    if not gateway_ip:
        print("无法从重定向URL提取网关IP")
        return
        
    print(f"使用网关IP: {gateway_ip}")

    # 检查是否为正确的认证页面
    try:
        login_page_response = session.get(redirect_url, timeout=5)
        login_page_response.encoding = 'utf-8'  # 确保使用UTF-8编码
        if not check_captive_portal_title(login_page_response.text):
            print("Captive portal标题与预期模式不匹配。")
            print("您可能未连接到正确的网络。")
            return
        else:
            print("检测到正确的认证门户页面")
    except Exception as e:
        print(f"获取登录页面失败: {e}")
        return

    # 提取magic参数
    magic = extract_magic_from_url(redirect_url)
    if not magic:
        print("无法提取magic参数。")
        return
        
    print(f"magic参数: {magic}")

    # 准备登录数据
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

    # 编码登录数据
    encoded_login_data = "&".join(
        f"{quote(k)}={quote(v)}" for k, v in login_data.items()
    )

    # 发送登录请求
    try:
        response = session.post(
            f"http://{gateway_ip}:1000/",
            data=encoded_login_data,
            headers=headers,
            timeout=30
        )
        
        # 检查登录是否成功
        if response.status_code == 200 and "/keepalive?" in response.text.lower():
            print("自动登录成功!")
        else:
            print("登录请求完成，但状态可能不成功")
            
    except requests.exceptions.RequestException as e:
        print(f"登录POST请求失败: {e}")

def main():
    print("NCUT校园网自动登录V2")
    print("by sangege & AI LIFE\n")
    print("https://github.com/apple050620312/NCUT-Internet-Auto-Login\n")
    
    # 设置登录信息
    global account, password
    account = "ncut"  # 替换为您的账号
    password = "ncut"    # 替换为您的密码
    
    # 在启动时显示账号和密码
    print(f"使用的账号: {account}")
    print(f"使用的密码: {password}\n")

    failed_attempts = 1

    while True:
        if not check_connection():
            failed_attempts += 1
            print(f"连接失败 (第{failed_attempts}次尝试)")
            
            if failed_attempts >= 5:
                print("尝试登录...")
                login()
                failed_attempts = 1
            time.sleep(2)
        else:
            if failed_attempts > 0:
                print("连接已恢复")
                failed_attempts = 0
            time.sleep(5)

if __name__ == "__main__":
    main()
