#pragma once

#include "ez_vulkan.h"
#include <string>
#include <vector>
#include <unordered_map>

void rhi_shader_mgr_init();

void rhi_shader_mgr_terminate();

void rhi_compile_shader(const std::string& file_path, void** data, uint32_t& size);

void rhi_compile_shader(const std::string& file_path, const std::vector<std::string>& macros, void** data, uint32_t& size);

EzShader rhi_get_shader(const std::string& file);

EzShader rhi_get_shader(const std::string& file, const std::vector<std::string>& macros);