#pragma once
#include <map>
#include <string>
#include <vector>

namespace Path {
std::map<std::string, std::string> get_protocols();

void register_protocol(const std::string& proto, const std::string& path);

std::string fix_path(const std::string& path);

std::string join(const std::string& base, const std::string& path);

std::string filename(const std::string& path);

std::string extension(const std::string& path);

std::string parent_path(const std::string& path);

std::vector<std::string> subdirs(const std::string& path);
}// namespace Path