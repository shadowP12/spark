#pragma once
#include "file_access_base.h"

class FileAccess : public FileAccessBase
{
public:
    typedef FileAccess*(*CreateFunc)();

    FileAccess() = default;

    virtual ~FileAccess() = default;

    virtual std::string get_path() const { return ""; }

    virtual std::string get_path_absolute() const { return ""; }

    static FileAccess* open(const std::string& path, int mode_flags, Error* out_err = nullptr);

    static bool exist(const std::string& name);

    template <class T>
    static void make_default()
    {
        _create_func = []()
        {
            return static_cast<FileAccess*>(new T());
        };
    }

protected:
    virtual Error _open(const std::string& path, int mode_flags) = 0;

private:
    static CreateFunc _create_func;
};