#include "binary_stream.h"

BinaryStream::BinaryStream(const std::vector<uint8_t>& data)
    : _data(data), _position(0) {}

void BinaryStream::write(const uint8_t* data, uint32_t size)
{
    _data.insert(_data.end(), data, data + size);
}

uint8_t* BinaryStream::read(uint32_t size)
{
    uint8_t* data_ptr = _data.data() + _position;
    _position += size;
    return data_ptr;
}