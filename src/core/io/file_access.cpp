#include "file_access.h"
#include "core/path.h"
#ifdef _WIN32
#include "platform/win/win_util.h"
#include <windows.h>
#include <errno.h>
#include <wchar.h>
#endif

FileAccess::CreateFunc FileAccess::_create_func = nullptr;

FileAccess* FileAccess::open(const std::string& path, int mode_flags, Error* out_err)
{
    FileAccess* ret = _create_func();
    Error err = ret->_open(path, mode_flags);
    if (out_err) {
        *out_err = err;
    }
    if (err != ERR_OK) {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool FileAccess::exist(const std::string& name)
{
    FileAccess* f = open(name, FileAccess::READ);
    if (!f)
        return false;
    delete f;
    return true;
}

#ifdef _WIN32
class FileAccessWindows : public FileAccess
{
public:
    FileAccessWindows() = default;

    virtual ~FileAccessWindows();

    virtual Error _open(const std::string& in_path, int mode_flags);

    virtual void close();

    virtual bool is_open() const;

    virtual std::string get_path() const;

    virtual std::string get_path_absolute() const;

    virtual void seek(uint64_t position);

    virtual void seek_end(int64_t position);

    virtual uint64_t get_position() const;

    virtual uint64_t get_length() const;

    virtual bool eof_reached() const;

    virtual uint8_t get_8() const;

    virtual uint64_t get_buffer(uint8_t* dst, uint64_t length) const;

    virtual void flush();

    virtual void store_8(uint8_t dest);

    virtual void store_buffer(const uint8_t* src, uint64_t length);

protected:
    FILE* f = nullptr;
    int flags = 0;
    std::string path;
    std::string path_src;
};

FileAccessWindows::~FileAccessWindows()
{
    close();
}

Error FileAccessWindows::_open(const std::string& in_path, int mode_flags)
{
    path_src = in_path;
    path = Path::fix_path(in_path);
    if (f)
    {
        close();
    }

    const WCHAR* mode_string;
    if (mode_flags == READ)
    {
        mode_string = L"rb";
    }
    else if (mode_flags == WRITE)
    {
        mode_string = L"wb";
    }
    else if (mode_flags == READ_WRITE)
    {
        mode_string = L"rb+";
    }
    else if (mode_flags == WRITE_READ)
    {
        mode_string = L"wb+";
    }
    else
    {
        return ERR_INVALID_PARAMETER;
    }

    f = _wfsopen((LPCWSTR)(WinUtil::convert_utf8_to_wide(path).c_str()), mode_string, _SH_DENYNO);

    Error err = ERR_OK;
    if (f == nullptr)
    {
        switch (errno)
        {
            case ENOENT:
                err = ERR_FILE_NOT_FOUND;
                break;
            default:
                err = ERR_FILE_CANT_OPEN;
                break;
        }
        return err;
    }
    else
    {
        flags = mode_flags;
        return err;
    }
}

void FileAccessWindows::close()
{
    if (!f)
        return;

    fclose(f);
    f = nullptr;
}

bool FileAccessWindows::is_open() const
{
    return (f != nullptr);
}

std::string FileAccessWindows::get_path() const
{
    return path_src;
}

std::string FileAccessWindows::get_path_absolute() const
{
    return path;
}

void FileAccessWindows::seek(uint64_t position)
{
    _fseeki64(f, (int64_t)position, SEEK_SET);
}

void FileAccessWindows::seek_end(int64_t position)
{
    _fseeki64(f, position, SEEK_END);
}

uint64_t FileAccessWindows::get_position() const
{
    int64_t position = _ftelli64(f);
    return position;
}

uint64_t FileAccessWindows::get_length() const
{
    int64_t position = _ftelli64(f);
    _fseeki64(f, 0, SEEK_END);
    int64_t size = _ftelli64(f);
    _fseeki64(f, position, SEEK_SET);

    return size;
}

bool FileAccessWindows::eof_reached() const
{
    if (feof(f))
    {
        return true;
    }
    return false;
}

uint8_t FileAccessWindows::get_8() const
{
    uint8_t b;
    fread(&b, 1, 1, f);
    return b;
}

uint64_t FileAccessWindows::get_buffer(uint8_t* dst, uint64_t length) const
{
    uint64_t read = fread(dst, 1, length, f);
    return read;
}

void FileAccessWindows::flush()
{
    fflush(f);
}

void FileAccessWindows::store_8(uint8_t dest)
{
    fwrite(&dest, 1, 1, f);
}

void FileAccessWindows::store_buffer(const uint8_t* src, uint64_t length)
{
    fwrite(src, 1, length, f);
}

struct FileAccessRegister
{
    FileAccessRegister()
    {
        FileAccess::make_default<FileAccessWindows>();
    }
};
FileAccessRegister file_access_register;

#endif