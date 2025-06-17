#include "win_util.h"
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
namespace WinUtil {
std::wstring convert_utf8_to_wide(const std::string& str_utf8)
{
    // Convert ansi to wide
    int count = MultiByteToWideChar(CP_ACP, 0, str_utf8.c_str(), (int)str_utf8.length(), nullptr, 0);
    std::wstring str_wide(count, 0);
    MultiByteToWideChar(CP_ACP, 0, str_utf8.c_str(), (int)str_utf8.length(), &str_wide[0], count);
    return str_wide;
}
}// namespace WinUtil
#endif