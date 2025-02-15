#include "rhi_shader_mgr.h"
#include "core/hash.h"
#include "core/path.h"
#include "core/io/file_access.h"

static std::unordered_map<std::size_t, EzShader> g_shader_dict;

void rhi_shader_mgr_init()
{
}

void rhi_shader_mgr_terminate()
{
    for (auto shader_iter : g_shader_dict)
    {
        ez_destroy_shader(shader_iter.second);
    }
    g_shader_dict.clear();
}

void rhi_compile_shader_internal(const std::string& file_path, const std::vector<std::string>& macros, void** data, uint32_t& size)
{
    std::string file_name = Path::filename(file_path);
    std::string parent_path = Path::parent_path(file_path);
    std::string log_file_path = parent_path + "/" + file_name + "_compile.log";
    std::string out_file_path = file_path + ".spv";

    std::string glslang_validator = getenv("VULKAN_SDK");
    glslang_validator += "/Bin/glslangValidator";

    std::string command_line;
    command_line += glslang_validator;
    char buff[256];
    sprintf(buff, R"( -V "%s" --target-env vulkan1.3 -o "%s")", file_path.c_str(), out_file_path.c_str());
    command_line += buff;
    for (const auto & macro : macros)
    {
        command_line += " \"-D" + macro + "\"";
    }
    sprintf(buff, " >\"%s\"", log_file_path.c_str());
    command_line += buff;

    if (std::system(command_line.c_str()) == 0)
    {
        FileAccess* fa = FileAccess::open(out_file_path, FileAccess::READ);
        size = fa->get_length();
        *data = malloc(size);
        fa->get_buffer((uint8_t*)*data, size);
        delete fa;
    }
}

void rhi_compile_shader(const std::string& file_path, void** data, uint32_t& size)
{
    std::string fix_path = Path::fix_path(file_path);
    std::vector<std::string> macros;
    rhi_compile_shader_internal(fix_path, macros, data, size);
}

void rhi_compile_shader(const std::string& file_path, const std::vector<std::string>& macros, void** data, uint32_t& size)
{
    std::string fix_path = Path::fix_path(file_path);
    rhi_compile_shader_internal(fix_path, macros, data, size);
}

EzShader rhi_get_shader_internal(const std::string& file, const std::vector<std::string>& macros)
{
    std::size_t hash = 0;
    hash_combine(hash, file);
    for (auto& macro : macros)
    {
        hash_combine(hash, macro);
    }

    auto iter = g_shader_dict.find(hash);
    if (iter != g_shader_dict.end())
    {
        return iter->second;
    }

    void* data = nullptr;
    uint32_t size;
    EzShader shader;
    rhi_compile_shader(file, macros, &data, size);
    ez_create_shader(data, size, shader);
    g_shader_dict[hash] = shader;
    if (data)
        free(data);
    return shader;
}

EzShader rhi_get_shader(const std::string& file)
{
    std::vector<std::string> macros;
    return rhi_get_shader_internal(file, macros);
}

EzShader rhi_get_shader(const std::string& file, const std::vector<std::string>& macros)
{
    return rhi_get_shader_internal(file, macros);
}