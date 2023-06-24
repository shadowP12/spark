#include "path.h"
#include <regex>

namespace Path
{
static std::map<std::string, std::string> protocols;

std::map<std::string, std::string> get_protocols()
{
    return protocols;
}

void register_protocol(const std::string& proto, const std::string& path)
{
    protocols[proto + "://"] = path;
}

std::string fix_path(const std::string& path)
{
    for (auto& proto : protocols)
    {
        if (path.find(proto.first) != std::string::npos)
        {
            return std::regex_replace(path, std::regex(proto.first), proto.second);
        }
    }
    return path;
}
}
