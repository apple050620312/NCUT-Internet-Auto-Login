#pragma once

#include <string>

namespace Net {

bool check_connection(int timeout_ms = 1000);

struct LoginResult {
    bool success{false};
    std::wstring message; // extra info
};

// Performs captive portal login using FortiGate-like flow
LoginResult login(const std::wstring& account, const std::wstring& password);

// Utilities exposed for potential diagnostics
std::wstring extract_redirect_url(const std::string& page_html);
std::wstring extract_gateway_ip(const std::wstring& redirect_url);
std::wstring extract_magic_from_url(const std::wstring& redirect_url);

}

