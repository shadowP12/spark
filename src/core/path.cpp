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

std::string join(const std::string& base, const std::string& path)
{
    return base + "/" + path;
}

std::string filename(const std::string& path)
{
    size_t pos = path.find_last_of("/");
    if (pos == std::string::npos)
        return path;
    return path.substr(pos+1);
}

std::string extension(const std::string& path)
{
    const std::string& name = filename(path);
    size_t pos = name.find_last_of(".");
    if (pos == std::string::npos)
        return "";
    return name.substr(pos+1);
}

std::string parent_path(const std::string& path)
{
    size_t pos = path.find_last_of("/");
    if (pos == std::string::npos)
        return "";
    return path.substr(0, pos);
}

std::vector<std::string> subdirs(const std::string& path)
{
    char delim = '/';
    std::size_t previous = 0;
    std::size_t current = path.find(delim);
    std::vector<std::string> elems;
    while (current != std::string::npos)
    {
        if (current > previous) {
            elems.push_back(path.substr(0, current+1));
        }
        previous = current + 1;
        current = path.find(delim, previous);
    }
    if (previous != path.size()) {
        elems.push_back(path);
    }
    return elems;
}
}
