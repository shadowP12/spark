#include "shader_compiler.h"
#include "core/path.h"
#include "core/io/file_access.h"

void ShaderCompiler::compile_internal(const std::string& file_path, const std::vector<std::string>& macros, void** data, uint32_t& size)
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
    sprintf(buff, R"( -V "%s" -o "%s")", file_path.c_str(), out_file_path.c_str());
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
        fa->get_buffer((uint8_t*)*data, size);
        delete fa;
    }
}

void ShaderCompiler::compile(const std::string& file_path, void** data, uint32_t& size)
{
    std::string fix_path = Path::fix_path(file_path);
    std::vector<std::string> macros;
    compile_internal(fix_path, macros, data, size);
}

void ShaderCompiler::compile(const std::string& file_path, const std::vector<std::string>& macros, void** data, uint32_t& size)
{
    std::string fix_path = Path::fix_path(file_path);
    compile_internal(fix_path, macros, data, size);
}