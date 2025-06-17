#pragma once

#include <vector>

class BinaryStream
{
public:
    BinaryStream() = default;

    explicit BinaryStream(const std::vector<uint8_t>& data);

    BinaryStream& operator=(const std::vector<uint8_t>& data)
    {
        _data = data;
        _position = 0;
        return *this;
    }

    BinaryStream& operator=(std::vector<uint8_t>&& data)
    {
        _data = std::move(data);
        _position = 0;
        return *this;
    }

    void write(const uint8_t* data, uint32_t size);

    uint8_t* read(uint32_t size);

    uint32_t get_size() { return _data.size(); }
    const uint8_t* get_data() { return _data.data(); }

private:
    uint32_t _position = 0;
    std::vector<uint8_t> _data;
};