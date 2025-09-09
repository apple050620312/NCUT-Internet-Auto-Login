#pragma once

#include <string>

namespace I18N {

void set_language(const std::wstring& code); // "en", "zh-TW"
const std::wstring& lang();
const std::wstring& t(const std::wstring& key);

}

