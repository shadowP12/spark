#include "dir_access.h"
#include "file_access.h"
#include "core/path.h"
#include <memory>
#include <vector>
#include <algorithm>
#ifdef _WIN32
#include "platform/win/win_util.h"
#include <windows.h>
#include <errno.h>
#include <wchar.h>
#endif

DirAccess::CreateFunc DirAccess::_create_func = nullptr;

bool DirAccess::dir_exists(const std::string& dir)
{
    std::shared_ptr<DirAccess> da = std::shared_ptr<DirAccess>(_create_func());
    return da->_dir_exists(dir);
}

Error DirAccess::make_dir(const std::string& dir)
{
    std::shared_ptr<DirAccess> da = std::shared_ptr<DirAccess>(_create_func());
    return da->_make_dir(dir);
}

Error DirAccess::make_dir_recursive(const std::string& dir)
{
    std::vector<std::string> subdirs = Path::subdirs(dir);
    for (int i = 0; i < subdirs.size(); ++i)
    {
        if (!dir_exists(subdirs[i])) {
            Error err = make_dir(subdirs[i]);
            if (err != ERR_OK && err != ERR_ALREADY_EXISTS) {
                return err;
            }
        }
    }
    return ERR_OK;
}

void DirAccess::copy(const std::string& from, const std::string& to)
{
    std::shared_ptr<FileAccess> fsrc = std::shared_ptr<FileAccess>(FileAccess::open(from, FileAccess::READ));
    std::shared_ptr<FileAccess> fdst = std::shared_ptr<FileAccess>(FileAccess::open(to, FileAccess::WRITE));

    fsrc->seek(0);
    uint64_t size = fsrc->get_length();
    uint64_t copy_buffer_limit = 65536; // 64 KB
    uint64_t buffer_size = std::min<uint64_t>(size * sizeof(uint8_t), copy_buffer_limit);
    std::vector<uint8_t> buffer;
    buffer.resize(buffer_size);

    while (size > 0)
    {
        uint64_t bytes_read = fsrc->get_buffer(buffer.data(), buffer_size);
        fdst->store_buffer(buffer.data(), bytes_read);
        size -= bytes_read;
    }
}

#ifdef _WIN32
class DirAccessWindows : public DirAccess
{
public:
    DirAccessWindows() = default;

    virtual bool _dir_exists(const std::string& dir);

    virtual Error _make_dir(const std::string& dir);
};

bool DirAccessWindows::_dir_exists(const std::string& dir)
{
    DWORD file_attr;
    file_attr = GetFileAttributesW((LPCWSTR)(WinUtil::convert_utf8_to_wide(dir).c_str()));
    if (INVALID_FILE_ATTRIBUTES == file_attr) {
        return false;
    }
    return (file_attr & FILE_ATTRIBUTE_DIRECTORY);
}

Error DirAccessWindows::_make_dir(const std::string& dir)
{
    bool success;
    int err;

    success = CreateDirectoryW((LPCWSTR)(WinUtil::convert_utf8_to_wide(dir).c_str()), nullptr);
    err = GetLastError();

    if (success) {
        return ERR_OK;
    }

    if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED) {
        return ERR_ALREADY_EXISTS;
    }

    return ERR_CANT_CREATE;
}

struct DirAccessRegister
{
    DirAccessRegister()
    {
        DirAccess::make_default<DirAccessWindows>();
    }
};
DirAccessRegister dir_access_register;
#endif