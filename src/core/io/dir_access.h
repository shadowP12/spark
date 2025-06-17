#pragma once

#include "core/error.h"
#include <string>

class DirAccess
{
public:
    typedef DirAccess* (*CreateFunc)();

    static bool dir_exists(const std::string& dir);

    static Error make_dir(const std::string& dir);

    static Error make_dir_recursive(const std::string& dir);

    static void copy(const std::string& from, const std::string& to);

    template<class T>
    static void make_default()
    {
        _create_func = []() {
            return static_cast<DirAccess*>(new T());
        };
    }

protected:
    virtual bool _dir_exists(const std::string& dir) = 0;

    virtual Error _make_dir(const std::string& dir) = 0;

private:
    static CreateFunc _create_func;
};