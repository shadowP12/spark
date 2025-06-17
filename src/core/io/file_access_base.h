#pragma once
#include "core/core.h"
#include "core/error.h"
#include <string>

class FileAccessBase
{
public:
    enum ModeFlags {
        READ = 1,
        WRITE = 2,
        READ_WRITE = 3,
        WRITE_READ = 7,
    };

    virtual void close() = 0;

    virtual bool is_open() const = 0;

    virtual void seek(uint64_t p_position) = 0;

    virtual void seek_end(int64_t p_position) = 0;

    virtual uint64_t get_position() const = 0;

    virtual uint64_t get_length() const = 0;

    virtual bool eof_reached() const = 0;

    virtual uint8_t get_8() const = 0;

    virtual uint16_t get_16() const;

    virtual uint32_t get_32() const;

    virtual uint64_t get_64() const;

    virtual float get_float() const;

    virtual double get_double() const;

    virtual uint64_t get_buffer(uint8_t* dst, uint64_t length) const = 0;

    virtual void flush() = 0;

    virtual void store_8(uint8_t p_dest) = 0;

    virtual void store_16(uint16_t dest);

    virtual void store_32(uint32_t dest);

    virtual void store_64(uint64_t dest);

    virtual void store_float(float dest);

    virtual void store_double(double dest);

    virtual void store_string(const std::string& str);

    virtual void store_buffer(const uint8_t* src, uint64_t length) = 0;

    bool big_endian = false;
};