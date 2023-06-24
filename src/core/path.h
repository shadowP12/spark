#pragma once
#include <map>
#include <string>

namespace Path
{
std::map<std::string, std::string> get_protocols();

void register_protocol(const std::string& proto, const std::string& path);

std::string fix_path(const std::string& path);
}