# ==============================================================================
# NCUT 校園網自動登入服務啟動器 (Windows Service 版本)
# 使用 pythonw.exe 運行，不顯示Console視窗
# ==============================================================================

import time
import socket
import urllib.request
import urllib.error
import re
import os
import sys
import logging
from http.cookiejar import CookieJar
from urllib.parse import quote
from datetime import datetime

# ==============================================================================
# 【帳號密碼設定區】
# ==============================================================================
account = "請替換為您的帳號並儲存（s+ 您的學號皆小寫）"
password = "請替換為您的密碼並儲存（身分證字號字母大寫）"
# ==============================================================================

# 設定日誌記錄
def setup_logging():
    """設定服務日誌記錄"""
    log_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)

    log_file = os.path.join(log_dir, f'ncut_login_{datetime.now().strftime("%Y%m%d")}.log')

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler(log_file, encoding='utf-8'),
            logging.StreamHandler()  # 同時輸出到 STDOUT 供 WinSW 記錄
        ]
    )
    return logging.getLogger(__name__)

logger = setup_logging()

def get_timestamp():
    """獲取當前時間戳記"""
    return datetime.now().strftime("[%Y-%m-%d %H:%M:%S]")

# 第一層實體網路偵測 (UDP 查表法)
def is_system_network_connected():
    """瞬間檢查系統實體網路狀態，0 毫秒延遲"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0] != "127.0.0.1"
    except Exception as e:
        logger.debug(f"Network check failed: {e}")
        return False

# 第二層登入狀態偵測
def check_login_status(max_retries=3):
    """
    檢查網路與登入狀態，區分「未登入」與「真斷線」
    回傳值：'ONLINE', 'NEEDS_LOGIN', 'UNSTABLE'
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

        except Exception as e:
            logger.debug(f"Login status check attempt {attempt + 1} failed: {e}")
            timeout_count += 1

        if timeout_count < max_retries:
            time.sleep(1.5)

    return 'UNSTABLE'

def extract_magic_from_url(url):
    """從 URL 中提取 magic 參數"""
    match = re.search(r"fgtauth\?([^&]+)", url)
    return match.group(1) if match else None

def extract_redirect_url(page_content):
    """從頁面內容中提取重新導向 URL"""
    match = re.search(
        r'window\.location\s*=\s*["\'](https?://[^"\']+/fgtauth\?[^"\']+)["\']',
        page_content,
    )
    return match.group(1) if match else None

def extract_gateway_ip(redirect_url):
    """從重新導向 URL 中提取閘道 IP 或主機名稱"""
    match = re.search(r"https?://([^/:]+)", redirect_url)
    if match:
        return match.group(1)
    return None

def check_captive_portal_title(page_content):
    """檢查是否為 NCUT 認證頁面"""
    title_pattern = r"勤益科技大學"
    match = re.search(title_pattern, page_content, re.IGNORECASE)
    return match is not None

def login():
    """執行登入操作"""
    logger.info("開始執行登入程序...")

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
        logger.error(f"[登入異常] 初始請求失敗無法取得重新導向：{e}")
        return

    # 提取重新導向 URL
    redirect_url = extract_redirect_url(initial_text)

    # [額外容錯] 若 Fortinet 直接給了 302 HTTP 跳轉
    if not redirect_url and "fgtauth" in initial_url:
        redirect_url = initial_url

    if not redirect_url:
        logger.error("[登入異常] 無法從頁面解析重新導向網址 (Redirect URL)。")
        return

    gateway_ip = extract_gateway_ip(redirect_url)
    if not gateway_ip:
        logger.error("[登入異常] 無法從重新導向網址解析閘道 IP(Gateway IP)。")
        return

    try:
        req = urllib.request.Request(redirect_url)
        with opener.open(req, timeout=5) as response:
            login_page_text = response.read().decode('utf-8', errors='ignore')
        if not check_captive_portal_title(login_page_text):
            logger.warning("[警告] 網頁標題不符，可能未連線到勤益校園網路。")
            return
    except Exception as e:
        logger.error(f"[登入異常] 取得登入頁面失敗：{e}")
        return

    # 提取 magic 參數
    magic = extract_magic_from_url(redirect_url)
    if not magic:
        logger.error("[登入異常] 無法提取認證 magic 參數。")
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
            logger.info("[登入成功] 已成功完成校園網路認證！")
        else:
            logger.warning("[登入異常] 登入請求完成，但未偵測到成功標記。")

    except urllib.error.URLError as e:
        if hasattr(e, 'read'):
            response_text = e.read().decode('utf-8', errors='ignore')
            if "/keepalive?" in response_text.lower():
                logger.info("[登入成功] 已成功完成校園網路認證！(帶有 HTTP 異常狀態)")
            else:
                logger.error(f"[登入失敗] POST 請求異常且無成功標記：{e}")
        else:
            logger.error(f"[登入失敗] POST 請求異常：{e}")

def main():
    """主函數"""
    logger.info("=" * 60)
    logger.info("NCUT 校園網自動登入服務 V3.0 (Windows Service 版本) 啟動")
    logger.info("Credit: hongfu553, AILIFE-4798, rileychh")
    logger.info("https://github.com/apple050620312/NCUT-Internet-Auto-Login")
    logger.info("=" * 60)

    # 首次執行初始化與自我覆寫設定
    global account, password

    if "請替換" in account or "請替換" in password or account == "" or password == "":
        logger.error("[錯誤] 帳號或密碼尚未設定！")
        logger.error("請在安裝時輸入帳號密碼，或手動修改此檔案。")

        # 嘗試從安裝配置文件讀取
        config_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'config.ini')
        if os.path.exists(config_file):
            try:
                with open(config_file, 'r', encoding='utf-8') as f:
                    lines = f.readlines()
                    if len(lines) >= 2:
                        account = lines[0].strip()
                        password = lines[1].strip()
                        logger.info(f"[設定載入] 帳號：{account}")
                        logger.info(f"[設定載入] 密碼：{'*' * len(password)}")
            except Exception as e:
                logger.error(f"[錯誤] 無法讀取配置文件：{e}")
                return
        else:
            logger.error(f"[錯誤] 配置文件不存在：{config_file}")
            return

    # 在啟動時顯示帳號和密碼 (密碼進行星號隱藏保護)
    logger.info(f"使用的帳號：{account}")
    logger.info(f"使用的密碼：{'*' * len(password)} (已隱藏保護)")
    logger.info("")

    last_state = 'INITIAL'

    while True:
        try:
            # 第一層：實體斷線防護
            if not is_system_network_connected():
                if last_state != 'SYSTEM_OFFLINE':
                    logger.warning("[用戶問題] 設備未連接網路，請檢查 Wi-Fi 或網路線是否已接上。")
                    last_state = 'SYSTEM_OFFLINE'
                time.sleep(3)
                continue

            status = check_login_status()

            if status == 'ONLINE':
                if last_state != 'ONLINE':
                    if last_state == 'INITIAL':
                        logger.info("[狀態] 網路連線正常且已登入！")
                    else:
                        logger.info("[網路恢復] 網路連線已恢復正常且已登入！")
                    last_state = 'ONLINE'
                time.sleep(5)

            elif status == 'NEEDS_LOGIN':
                if last_state != 'NEEDS_LOGIN':
                    logger.info("[狀態變更] 偵測到未登入狀態或授權過期，準備執行自動登入程序...")
                    last_state = 'NEEDS_LOGIN'
                login()
                time.sleep(2)

            elif status == 'UNSTABLE':
                if last_state != 'UNSTABLE':
                    logger.warning("[學校網路問題] 已連接到網路，但無法存取外網且無認證頁面。")
                    last_state = 'UNSTABLE'
                time.sleep(3)

        except KeyboardInterrupt:
            logger.info("[服務] 收到中斷訊號，正在停止...")
            break
        except Exception as e:
            logger.error(f"[異常] 主迴圈發生意外錯誤：{e}")
            time.sleep(5)

if __name__ == "__main__":
    main()