#pragma once

#ifdef _WIN32
#include <string>

namespace WinUtil
{
std::wstring convert_utf8_to_wide(const std::string& str_utf8);
}
#endif