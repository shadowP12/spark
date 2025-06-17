#include "file_access_base.h"

union MarshallFloat {
    uint32_t i;
    float f;
};

union MarshallDouble {
    uint64_t l;
    double d;
};

uint16_t FileAccessBase::get_16() const
{
    uint16_t ret;
    uint8_t a, b;

    a = get_8();
    b = get_8();

    if (big_endian)
    {
        SWAP(a, b);
    }

    ret = b;
    ret <<= 8;
    ret |= a;

    return ret;
}

uint32_t FileAccessBase::get_32() const
{
    uint32_t ret;
    uint16_t a, b;

    a = get_16();
    b = get_16();

    if (big_endian)
    {
        SWAP(a, b);
    }

    ret = b;
    ret <<= 16;
    ret |= a;

    return ret;
}

uint64_t FileAccessBase::get_64() const
{
    uint64_t ret;
    uint32_t a, b;

    a = get_32();
    b = get_32();

    if (big_endian)
    {
        SWAP(a, b);
    }

    ret = b;
    ret <<= 32;
    ret |= a;

    return ret;
}

float FileAccessBase::get_float() const
{
    MarshallFloat m{};
    m.i = get_32();
    return m.f;
}

double FileAccessBase::get_double() const
{
    MarshallFloat m{};
    m.i = get_32();
    return m.f;
}

void FileAccessBase::store_16(uint16_t dest)
{
    uint8_t a, b;

    a = dest & 0xFF;
    b = dest >> 8;

    if (big_endian)
    {
        SWAP(a, b);
    }

    store_8(a);
    store_8(b);
}

void FileAccessBase::store_32(uint32_t dest)
{
    uint16_t a, b;

    a = dest & 0xFFFF;
    b = dest >> 16;

    if (big_endian)
    {
        SWAP(a, b);
    }

    store_16(a);
    store_16(b);
}

void FileAccessBase::store_64(uint64_t dest)
{
    uint32_t a, b;

    a = dest & 0xFFFFFFFF;
    b = dest >> 32;

    if (big_endian)
    {
        SWAP(a, b);
    }

    store_32(a);
    store_32(b);
}

void FileAccessBase::store_float(float dest)
{
    MarshallFloat m{};
    m.f = dest;
    store_32(m.i);
}

void FileAccessBase::store_double(double dest)
{
    MarshallDouble m{};
    m.d = dest;
    store_64(m.l);
}

void FileAccessBase::store_string(const std::string& str)
{
    store_buffer((const uint8_t*)str.data(), str.size());
}