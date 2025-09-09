#include "Network.h"
#include "Logger.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winhttp.h>

#include <string>
#include <regex>
#include <vector>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winhttp.lib")

using namespace std;

namespace Net {

static bool init_winsock() {
    static bool initialized = false;
    static WSADATA wsaData;
    if (!initialized) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return false;
        }
        initialized = true;
    }
    return true;
}

bool check_connection(int timeout_ms) {
    if (!init_winsock()) return false;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return false;

    // Make non-blocking for custom timeout
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    inet_pton(AF_INET, "1.1.1.1", &addr.sin_addr);

    int result = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);
            timeval tv{};
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            result = select(0, nullptr, &writeSet, nullptr, &tv);
            if (result > 0) {
                // Check for errors
                int so_error = 0; int len = sizeof(so_error);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);
                closesocket(sock);
                return so_error == 0;
            } else {
                closesocket(sock);
                return false;
            }
        } else {
            closesocket(sock);
            return false;
        }
    }

    closesocket(sock);
    return true;
}

static string http_get(const wstring& url) {
    // Simple WinHTTP GET, returns body as UTF-8 (best effort)
    URL_COMPONENTS comps{};
    wchar_t host[256]{}; wchar_t path[2048]{};
    comps.dwStructSize = sizeof(comps);
    comps.lpszHostName = host; comps.dwHostNameLength = _countof(host);
    comps.lpszUrlPath = path; comps.dwUrlPathLength = _countof(path);

    if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.size(), 0, &comps)) {
        return {};
    }

    INTERNET_PORT port = comps.nPort;
    bool secure = comps.nScheme == INTERNET_SCHEME_HTTPS;

    HINTERNET hSession = WinHttpOpen(L"NCUTAutoLogin/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return {};
    HINTERNET hConnect = WinHttpConnect(hSession, host, port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return {}; }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return {}; }

    string body;
    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
        && WinHttpReceiveResponse(hRequest, nullptr);
    if (ok) {
        DWORD avail = 0;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &avail)) break;
            if (avail == 0) break;
            vector<char> buf(avail);
            DWORD read = 0;
            if (!WinHttpReadData(hRequest, buf.data(), avail, &read) || read == 0) break;
            body.append(buf.data(), read);
        } while (avail > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return body;
}

static wstring to_wlower(const wstring& s) {
    wstring t = s;
    transform(t.begin(), t.end(), t.begin(), ::towlower);
    return t;
}

wstring extract_redirect_url(const string& page_html) {
    // Look for window.location="http://x.x.x.x:1000/fgtauth?XXXX"
    try {
        regex re("window\\.location=\\\"(http://\\d+\\.\\d+\\.\\d+\\.\\d+:1000/fgtauth\\?[^\\\"]+)\\\"");
        smatch m;
        if (regex_search(page_html, m, re) && m.size() > 1) {
            string u8 = m[1].str();
            int wlen = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), (int)u8.size(), nullptr, 0);
            wstring w(wlen, 0);
            MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), (int)u8.size(), &w[0], wlen);
            return w;
        }
    } catch (...) {
    }
    return L"";
}

wstring extract_gateway_ip(const wstring& redirect_url) {
    try {
        wregex re(L"http://(\\d+\\.\\d+\\.\\d+\\.\\d+):1000");
        wsmatch m;
        if (regex_search(redirect_url, m, re) && m.size() > 1) {
            return m[1].str();
        }
    } catch (...) {
    }
    return L"";
}

wstring extract_magic_from_url(const wstring& redirect_url) {
    // Match everything after fgtauth? until '&' or end
    try {
        wregex re(L"fgtauth\\?([^&]+)");
        wsmatch m;
        if (regex_search(redirect_url, m, re) && m.size() > 1) {
            return m[1].str();
        }
    } catch (...) {
    }
    return L"";
}

static bool http_post_form(const wstring& host, INTERNET_PORT port, const wstring& path, const wstring& origin, const wstring& referer, const string& body, string& out) {
    bool secure = false;
    HINTERNET hSession = WinHttpOpen(L"NCUTAutoLogin/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    wstring headers = L"Content-Type: application/x-www-form-urlencoded\r\n";
    headers += L"Upgrade-Insecure-Requests: 1\r\n";
    if (!referer.empty()) {
        headers += L"Referer: "; headers += referer; headers += L"\r\n";
    }
    if (!origin.empty()) {
        headers += L"Origin: "; headers += origin; headers += L"\r\n";
    }

    BOOL ok = WinHttpSendRequest(
        hRequest,
        headers.c_str(), (DWORD)headers.size(),
        (LPVOID)body.data(), (DWORD)body.size(),
        (DWORD)body.size(), 0);

    if (ok) ok = WinHttpReceiveResponse(hRequest, nullptr);
    if (ok) {
        DWORD avail = 0;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &avail)) break;
            if (avail == 0) break;
            vector<char> buf(avail);
            DWORD read = 0;
            if (!WinHttpReadData(hRequest, buf.data(), avail, &read) || read == 0) break;
            out.append(buf.data(), read);
        } while (avail > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return ok;
}

static string url_encode(const wstring& w) {
    // naive percent-encode for ASCII; for non-ASCII we UTF-8 encode then percent-encode
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    string s(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), len, nullptr, nullptr);
    static const char* hex = "0123456789ABCDEF";
    string out;
    for (unsigned char c : s) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='-'||c=='_'||c=='.'||c=='~') {
            out.push_back((char)c);
        } else {
            out.push_back('%');
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        }
    }
    return out;
}

LoginResult login(const wstring& account, const wstring& password) {
    LoginResult result;
    Logger::instance().info(L"嘗試觸發登入流程...");

    // 1) GET to gstatic to trigger captive portal intercept
    string page = http_get(L"http://www.gstatic.com/generate_204");
    if (page.empty()) {
        result.message = L"初始請求失敗";
        return result;
    }

    // 2) Extract redirect URL
    wstring redirect_url = extract_redirect_url(page);
    if (redirect_url.empty()) {
        result.message = L"無法解析重導URL";
        return result;
    }
    Logger::instance().info(L"取得重導URL: " + redirect_url);

    // 3) Extract gateway IP
    wstring gateway_ip = extract_gateway_ip(redirect_url);
    if (gateway_ip.empty()) {
        result.message = L"無法取得閘道IP";
        return result;
    }
    Logger::instance().info(L"閘道IP: " + gateway_ip);

    // 4) GET the login page (optional check)
    string login_page = http_get(redirect_url);
    if (login_page.empty()) {
        result.message = L"取得登入頁面失敗";
        return result;
    }

    // 5) Extract magic
    wstring magic = extract_magic_from_url(redirect_url);
    if (magic.empty()) {
        result.message = L"無法取得 magic 參數";
        return result;
    }
    Logger::instance().info(L"magic: " + magic);

    // 6) POST credentials
    wstring origin = L"http://" + gateway_ip + L":1000";
    wstring referer = redirect_url;
    string body;
    body += "4Tredir=" + string("http%3A%2F%2Fwww.gstatic.com%2Fgenerate_204");
    body += "&magic=" + url_encode(magic);
    body += "&username=" + url_encode(account);
    body += "&password=" + url_encode(password);

    string resp;
    bool ok = http_post_form(gateway_ip, 1000, L"/", origin, referer, body, resp);
    if (!ok) {
        result.message = L"登入請求失敗";
        return result;
    }

    // 7) Check success
    string low = resp;
    transform(low.begin(), low.end(), low.begin(), ::tolower);
    if (low.find("/keepalive?") != string::npos) {
        result.success = true;
        result.message = L"登入成功";
        return result;
    } else {
        result.message = L"登入未成功，請檢查帳密";
        return result;
    }
}

} // namespace Net

